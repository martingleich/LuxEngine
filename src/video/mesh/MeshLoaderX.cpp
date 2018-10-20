#include "MeshLoaderX.h"
#include "math/Matrix4.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"
#include "video/MaterialLibrary.h"
#include "video/Texture.h"
#include "io/FileSystem.h"
#include "io/File.h"
#include "core/Logger.h"
#include "core/StringConverter.h"
#include "core/lxException.h"
#include "core/lxGUID.h"

// TODO: Merging of skinning data.

namespace lux
{
namespace video
{
namespace xfile_loader_impl
{
core::String g_MeshLoaderName = "lux xfile loader";
core::HashMap<core::String, core::GUID> g_TypeGUIDS;
core::GUID g_GUIDFrame;
core::GUID g_GUIDFrameTransformMatrix;
core::GUID g_GUIDMesh;
core::GUID g_GUIDMeshNormals;
core::GUID g_GUIDMeshTextureCoords;
core::GUID g_GUIDMeshMaterialList;
core::GUID g_GUIDSkinWeights;
core::GUID g_GUIDMaterial;
core::GUID g_GUIDTextureFileName;

video::VertexFormat g_VertexFormatSkinned;

class XFileFormatException : public core::FileFormatException
{
public:
	XFileFormatException(core::StringView _msg) :
		FileFormatException(_msg, "xfile")
	{
	}
};

enum class EError
{
	InvalidMagic,
	UnsupportedVersion,
	UnsupportedFormat,
	UnsupportedFloatSize,
	UnsupportedTokenType,
};

class ErrorReporter
{
public:
	void Error(EError e) { LUX_UNUSED(e); }
	void Warning(const char* msg, const char* data) { LUX_UNUSED(msg, data); }
};

struct Header
{
	int majorVersion;
	int minorVersion;
	bool binary;
	bool compressed;
	bool doublePrecision;
};

enum EToken
{
	TOKEN_INVALID = 0,

	// Record token
	TOKEN_NAME = 1,
	TOKEN_STRING = 2,
	TOKEN_INTEGER = 3,
	TOKEN_GUID = 5,
	TOKEN_INTEGER_LIST = 6,
	TOKEN_FLOAT_LIST = 7,

	// Stand alone token
	TOKEN_OBRACE = 10,
	TOKEN_CBRACE = 11,
	TOKEN_OPAREN = 12,
	TOKEN_CPAREN = 13,
	TOKEN_OBRACKET = 14,
	TOKEN_CBRACKET = 15,
	TOKEN_OANGLE = 16,
	TOKEN_CANGLE = 17,
	TOKEN_DOT = 18,
	TOKEN_COMMA = 19,
	TOKEN_SEMICOLON = 20,
	TOKEN_TEMPLATE = 31,
	TOKEN_WORD = 40,
	TOKEN_DWORD = 41,
	TOKEN_FLOAT = 42,
	TOKEN_DOUBLE = 43,
	TOKEN_CHAR = 44,
	TOKEN_UCHAR = 45,
	TOKEN_SWORD = 46,
	TOKEN_SDWORD = 47,
	TOKEN_VOID = 48,
	TOKEN_LPSTR = 49,
	TOKEN_UNICODE = 50,
	TOKEN_CSTRING = 51,
	TOKEN_ARRAY = 52,

	TOKEN_EOF = 0xFF,
};

core::HashMap<core::String, EToken> g_KeywordMap;

class Parser;
class Token
{
public:
	Token(EToken _type, Parser* _parser) :
		type(_type),
		parser(_parser)
	{
	}
	Token() :
		type(TOKEN_INVALID),
		parser(nullptr)
	{
	}

	const core::String& GetString() const;
	EToken GetStringTerminator() const;
	const core::String& GetName() const;
	const core::String& GetNameUpper() const;

	const core::GUID& GetGuid() const;

	EToken TryMatch(EToken tok1, EToken tok2 = TOKEN_INVALID, EToken tok3 = TOKEN_INVALID) const
	{
		if(type == tok1 || type == tok2 || type == tok3)
			return type;
		return TOKEN_INVALID;
	}
	EToken Match(EToken tok1, EToken tok2 = TOKEN_INVALID, EToken tok3 = TOKEN_INVALID) const
	{
		EToken t = TryMatch(tok1, tok2, tok3);
		if(t == TOKEN_INVALID)
			throw XFileFormatException("Other token excpected");
		return t;
	}

	EToken type;

private:
	Parser* parser;
};

class Parser
{
	friend class Token;
public:
	void Reset(io::File* file, ErrorReporter* errorRep)
	{
		m_File = file;
		m_ErrRep = errorRep;
		m_CurToken = Token(TOKEN_INVALID, this);
		m_DataBuffer.SetMinSize(4);

		m_ListRemain = 0;
		m_PushedChar = 0;
		m_DelayedRead = 0;
	}

	bool ReadHeader()
	{
		char data[16];
		m_File->ReadBinary(sizeof(data), data);
		if(memcmp(data, "xof ", 4) != 0) {
			m_ErrRep->Error(EError::InvalidMagic);
			return false;
		}

		m_Header.majorVersion = data[5] - '0';
		m_Header.minorVersion = data[7] - '0';
		if(m_Header.majorVersion != 3 || m_Header.minorVersion < 0 || m_Header.minorVersion > 9) {
			m_ErrRep->Error(EError::UnsupportedVersion);
			return false;
		}

		if(memcmp(data + 8, "txt ", 4) == 0) {
			m_Header.binary = false;
		} else if(memcmp(data + 8, "bin ", 4) == 0) {
			m_Header.binary = true;
		} else {
			m_ErrRep->Error(EError::UnsupportedFormat);
			return false;
		}

		if(memcmp(data + 12, "0032", 4) == 0) {
			m_Header.doublePrecision = false;
		} else if(memcmp(data + 12, "0064", 4) == 0) {
			m_Header.doublePrecision = true;
		} else {
			m_ErrRep->Error(EError::UnsupportedFloatSize);
			return false;
		}

		return true;
	}

	Token NextToken()
	{
		while(m_DelayedRead) {
			--m_DelayedRead;
			NextToken();
		}

		if(m_Header.binary) {
			if(m_ListRemain) {
				int bytePerObj = m_Header.doublePrecision ? 8 : 4;
				m_File->Seek(m_ListRemain*bytePerObj);
				m_ListRemain = 0;
			}

			u16 value;
			m_File->ReadBinary(2, &value);
			m_CurToken = Token(GetToken(value), this);
			ParseBinaryToken();
		} else {
			m_CurToken = ReadTextToken(m_DataBuffer);
		}

		return m_CurToken;
	}

	void NextTokenSilent()
	{
		++m_DelayedRead;
	}

	Token CurrentToken()
	{
		while(m_DelayedRead) {
			--m_DelayedRead;
			NextToken();
		}

		return m_CurToken;
	}

	void MatchTextOnly(EToken tok)
	{
		if(!m_Header.binary) {
			CurrentToken().Match(tok);
			NextTokenSilent();
		}
	}

	template <typename T>
	void ReadArray(u32 size, const T& func)
	{
		if(!m_Header.binary) {
			for(u32 i = 0; i < size;) {
				func(i);
				++i;
				if(i != size) {
					CurrentToken().Match(TOKEN_COMMA);
					NextToken();
				}
			}
			CurrentToken().Match(TOKEN_SEMICOLON);
			NextToken();
		} else {
			for(u32 i = 0; i < size; ++i)
				func(i);
		}
	}

	const float* ReadFloatArray(u32 count, void* dst)
	{
		auto out = (const float*)ReadBaseList(TOKEN_FLOAT, count, true, dst);
		MatchTextOnly(TOKEN_SEMICOLON);
		return out;
	}

	const float* ReadFloatElements(u32 count, void* dst)
	{
		return (const float*)ReadBaseList(TOKEN_FLOAT, count, false, dst);
	}

	const u32* ReadDWORDArray(u32 count, void* dst = nullptr)
	{
		auto out = (const u32*)ReadBaseList(TOKEN_INTEGER, count, true, dst);
		MatchTextOnly(TOKEN_SEMICOLON);
		return out;
	}

	u32 ReadDWORD()
	{
		u32 out = *(const u32*)ReadBaseList(TOKEN_INTEGER, 1, true, nullptr);
		MatchTextOnly(TOKEN_SEMICOLON);
		return out;
	}

	core::String ReadString()
	{
		CurrentToken().Match(TOKEN_STRING);
		auto out = CurrentToken().GetString();
		NextToken();
		MatchTextOnly(TOKEN_SEMICOLON);
		return out;
	}

	bool IsEOF() const
	{
		return m_File->IsEOF();
	}

private:
	// Begins on token in list, ends on token after list.
	const void* ReadBaseList(EToken type, u32 count, bool isArray, void* dst)
	{
		if(!dst) {
			m_DataBuffer.SetMinSize(count * 4, core::RawMemory::COPY);
			dst = m_DataBuffer;
		}

		if(m_Header.binary) {
			CurrentToken().Match(TOKEN_INTEGER_LIST, TOKEN_INTEGER, TOKEN_FLOAT_LIST);

			u8* cur = (u8*)dst;
			while(count) {
				if(m_ListRemain) {
					u32 read = math::Min(m_ListRemain, count);
					if(m_Header.doublePrecision) {
						for(u32 i = 0; i < read; ++i) {
							double value;
							m_File->ReadBinary(8, &value);
							*(float*)cur = static_cast<float>(value);
							cur += 4;
						}
					} else {
						m_File->ReadBinary(4 * read, cur);
					}
					cur += read * 4;
					count -= read;
					m_ListRemain -= read;
				}

				if(!m_ListRemain && count)
					NextToken();
			}

			// If done with this list, go to the next token.
			if(!m_ListRemain)
				NextTokenSilent();
			return dst;
		} else {
			EToken internalSeperator = isArray ? TOKEN_COMMA : TOKEN_SEMICOLON;
			CurrentToken().Match(type);
			memcpy(dst, m_DataBuffer, 4);
			for(u32 i = 1; i < count; ++i) {
				if(i != count)
					ReadTextToken().Match(internalSeperator);
				ReadTextToken((u32*)dst + i).Match(type);
			}
			if(!isArray)
				ReadTextToken().Match(TOKEN_SEMICOLON);
			NextTokenSilent();

			return (int*)dst;
		}
	}

	void ParseBinaryToken()
	{
		u32 count;
		switch(m_CurToken.type) {
		case TOKEN_STRING:
		case TOKEN_NAME:
			m_File->ReadBinary(4, &count);
			m_DataString.Resize(count);
			m_File->ReadBinary(count, m_DataString.Data());
			if(m_CurToken.type == TOKEN_STRING) {
				u16 token;
				m_File->ReadBinary(2, &token);
				m_DataStringTerminator = GetToken(token);
			}
			m_DataStringUpper = m_DataString.GetUpper();
			break;
		case TOKEN_GUID:
			m_File->ReadBinary(16, &m_DataGuid);
			break;
		case TOKEN_FLOAT_LIST:
		case TOKEN_INTEGER_LIST:
			m_File->ReadBinary(4, &m_ListRemain);
			break;
		case TOKEN_INTEGER:
			m_ListRemain = 1;
			break;
		default:
			break;
		}
	}

	Token ReadTextToken(void* valueDstBuffer = nullptr)
	{
		while(m_DelayedRead) {
			--m_DelayedRead;
			ReadTextToken(valueDstBuffer);
		}

		if(m_File->IsEOF()) {
			m_CurToken.type = TOKEN_EOF;
			return m_CurToken;
		}

		float garbageArea;
		if(!valueDstBuffer)
			valueDstBuffer = &garbageArea;

		int c = ReadChar();

		// Ignore whitespace and comments
		bool found;
		do {
			found = false;
			while((c == ' ' || c == '\t' || c == '\r' || c == '\n') && c != -1) {
				c = ReadChar();
				found = true;
			}
			while(c == '#' || (c == '/' && PeekChar() == '/') && c != -1) {
				while(c != '\r' && c != '\n' && c != -1)
					c = ReadChar();
				found = true;
			}
		} while(found);
		if(c == -1) {
			m_CurToken.type = TOKEN_EOF;
			return m_CurToken;
		}

		if(c == '-' || c == '+' || (c >= '0' && c <= '9')) {
			bool sign = false;
			if(c == '-') {
				sign = true;
				c = ReadChar();
			} else if(c == '+') {
				sign = false;
				c = ReadChar();
			}
			int iValue = 0;
			while(c >= '0' && c <= '9') {
				iValue *= 10;
				iValue += c - '0';
				c = ReadChar();
			}
			if(c == '.') {
				c = ReadChar();
				double dValue = iValue;
				iValue = 0;
				int power = 1;
				while(c >= '0' && c <= '9') {
					iValue *= 10;
					iValue += c - '0';
					power *= 10;
					c = ReadChar();
				}
				dValue += (double)iValue / power;
				dValue = sign ? -dValue : dValue;
				m_CurToken.type = TOKEN_FLOAT;
				*((float*)valueDstBuffer) = (float)dValue;
			} else {
				m_CurToken.type = TOKEN_INTEGER;
				iValue = sign ? -iValue : iValue;
				*((int*)valueDstBuffer) = iValue;
			}
			PushChar(c);
			return m_CurToken;
		}

		switch(c) {
		case '(': m_CurToken.type = TOKEN_OPAREN; break;
		case ')': m_CurToken.type = TOKEN_CPAREN; break;
		case '{': m_CurToken.type = TOKEN_OBRACE; break;
		case '}': m_CurToken.type = TOKEN_CBRACE; break;
		case '[': m_CurToken.type = TOKEN_OBRACKET; break;
		case ']': m_CurToken.type = TOKEN_CBRACKET; break;
		case '<':
			m_DataString.Clear();
			m_CurToken.type = TOKEN_OANGLE;
			c = ReadChar();
			while(c != '>' && c != -1) {
				m_DataString.AppendByte((u8)c);
				c = ReadChar();
			}
			if(m_DataString.Size() != 36) {
				m_CurToken.type = TOKEN_INVALID;
				break;
			}
			m_DataGuid = core::GUID::FromString(m_DataString);
			break;
		case '.': m_CurToken.type = TOKEN_DOT; break;
		case ',': m_CurToken.type = TOKEN_COMMA; break;
		case ';': m_CurToken.type = TOKEN_SEMICOLON; break;
		case '"':
			// String value
		{
			c = ReadChar();
			m_DataString.Clear();
			while(c != '"' && c != -1) {
				m_DataString.AppendByte((u8)c);
				c = ReadChar();
			}
			m_CurToken.type = TOKEN_STRING;
			break;
		}
		default:
			if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')) {
				m_CurToken.type = TOKEN_INVALID;
				break;
			}
			m_DataString.Clear();
			while((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
				m_DataString.AppendByte((u8)c);
				c = ReadChar();
			}
			PushChar(c);

			m_DataStringUpper = m_DataString.GetUpper();
			m_CurToken.type = g_KeywordMap.Get(m_DataStringUpper, TOKEN_NAME);
			break;
		}

		return m_CurToken;
	}

	static EToken GetToken(u16 value)
	{
		switch(value) {
		case 0: return TOKEN_INVALID;
		case 1: return TOKEN_NAME;
		case 2: return TOKEN_STRING;
		case 3: return TOKEN_INTEGER;
		case 5: return TOKEN_GUID;
		case 6: return TOKEN_INTEGER_LIST;
		case 7: return TOKEN_FLOAT_LIST;

		case 10: return TOKEN_OBRACE;
		case 11: return TOKEN_CBRACE;
		case 12: return TOKEN_OPAREN;
		case 13: return TOKEN_CPAREN;
		case 14: return TOKEN_OBRACKET;
		case 15: return TOKEN_CBRACKET;
		case 16: return TOKEN_OANGLE;
		case 17: return TOKEN_CANGLE;
		case 18: return TOKEN_DOT;
		case 19: return TOKEN_COMMA;
		case 20: return TOKEN_SEMICOLON;
		case 31: return TOKEN_TEMPLATE;
		case 40: return TOKEN_WORD;
		case 41: return TOKEN_DWORD;
		case 42: return TOKEN_FLOAT;
		case 43: return TOKEN_DOUBLE;
		case 44: return TOKEN_CHAR;
		case 45: return TOKEN_UCHAR;
		case 46: return TOKEN_SWORD;
		case 47: return TOKEN_SDWORD;
		case 48: return TOKEN_VOID;
		case 49: return TOKEN_LPSTR;
		case 50: return TOKEN_UNICODE;
		case 51: return TOKEN_CSTRING;
		case 52: return TOKEN_ARRAY;
		default: return TOKEN_INVALID;
		}
	}

	void PushChar(int c)
	{
		lxAssert(!m_PushedChar);
		m_PushedChar = c;
	}
	int PeekChar()
	{
		m_PushedChar = ReadChar();
		return m_PushedChar;
	}
	int ReadChar()
	{
		if(m_PushedChar) {
			int c = m_PushedChar;
			m_PushedChar = 0;
			return c;
		}
		char out;
		auto count = m_File->ReadBinaryPart(1, &out);
		if(!count)
			return -1;
		else
			return out;
	}

private:
	ErrorReporter* m_ErrRep;

	io::File* m_File;
	Header m_Header;
	Token m_CurToken;

	core::String m_DataString;
	core::String m_DataStringUpper;
	EToken m_DataStringTerminator;
	core::RawMemory m_DataBuffer;

	core::GUID m_DataGuid;

	u32 m_ListRemain;
	int m_PushedChar;
	int m_DelayedRead;
};

const core::String& Token::GetString() const
{
	Match(TOKEN_STRING);
	return parser->m_DataString;
}
EToken Token::GetStringTerminator() const
{
	Match(TOKEN_STRING);
	return parser->m_DataStringTerminator;
}
const core::String& Token::GetName() const
{
	Match(TOKEN_NAME);
	return parser->m_DataString;
}
const core::String& Token::GetNameUpper() const
{
	Match(TOKEN_NAME);
	return parser->m_DataStringUpper;
}
const core::GUID& Token::GetGuid() const
{
	Match(TOKEN_GUID);
	return parser->m_DataGuid;
}

class XFileTraverser;
struct Scope
{
	core::GUID type;
	core::GUID identGUID;
	core::String identName;
};

class XFileVisitor
{
public:
	XFileVisitor(XFileTraverser* t, Parser* p) :
		traverser(t),
		parser(p)
	{
	}

	virtual ~XFileVisitor() {}

	virtual void EnterScope(const Scope& scope) = 0;
	virtual bool OnObject(const Scope& scope) = 0;
	virtual void LeaveScope(const Scope& scope) = 0;

	XFileTraverser* traverser;
	Parser* parser;
};

class XFileTraverser
{
public:
	static const int IGNORE = 1;
	static const int RESTRICTED = 2;

	void Traverse(XFileVisitor* visitor, io::File* file, ErrorReporter* errRep)
	{
		m_Visitor = visitor;

		m_Break = false;
		m_Parser.Reset(file, errRep);

		m_Parser.ReadHeader();

		while(!m_Break) {
			auto token = m_Parser.NextToken();
			if(token.type == TOKEN_NAME) {
				ReadObject();
			} else if(token.type == TOKEN_TEMPLATE) {
				m_Parser.NextToken().Match(TOKEN_NAME);
				m_Parser.NextToken().Match(TOKEN_OBRACE);
				m_Parser.NextToken();
				ReadOpenObjectData(IGNORE);
			} else if(token.type == TOKEN_EOF) {
				break;
			} else {
				throw XFileFormatException("File corrupted");
			}
		}
	}

	// Begines on name, ends on CBRACE
	void ReadObject(int flags = 0, const core::GUID& restricted = core::GUID::EMPTY)
	{
		Scope scope;

		auto tok = m_Parser.CurrentToken();
		scope.type = g_TypeGUIDS.Get(tok.GetNameUpper(), core::GUID::EMPTY);

		if(TestFlag(flags, RESTRICTED) && scope.type != restricted)
			throw XFileFormatException("Invalid guid in restricted area");

		tok = m_Parser.NextToken();
		if(tok.type == TOKEN_NAME) {
			scope.identName = tok.GetName();
			tok = m_Parser.NextToken();
		}

		tok.Match(TOKEN_OBRACE);
		if(m_Break) {
			m_Parser.NextToken();
			ReadOpenObjectData(IGNORE);
			m_Parser.CurrentToken().Match(TOKEN_CBRACE);
			return;
		}

		tok = m_Parser.NextToken();
		if(tok.type == TOKEN_GUID) {
			scope.identGUID = tok.GetGuid();
			tok = m_Parser.NextToken();
		}

		m_Visitor->EnterScope(scope);
		lxAssert(m_Break == false);
		if(!m_Break && !m_Visitor->OnObject(scope))
			ReadOpenObjectData(XFileTraverser::IGNORE);

		m_Parser.CurrentToken().Match(TOKEN_CBRACE);
		m_Visitor->LeaveScope(scope);
	}

	// Begins on token inside of data, end on CBRACE
	void ReadOpenObjectData(int flags = 0, const core::GUID& restricted = core::GUID::EMPTY)
	{
		int level = 1;
		Token tok = m_Parser.CurrentToken();
		while(true) {
			if(m_Break)
				flags |= IGNORE;
			if(tok.type == TOKEN_OBRACE)
				++level;
			else if(tok.type == TOKEN_CBRACE)
				--level;
			else if(!TestFlag(flags, IGNORE) && tok.type == TOKEN_NAME)
				ReadObject(flags, restricted);
			else if(tok.type == TOKEN_INVALID)
				throw XFileFormatException("File is corrupted.");

			if(level == 0)
				break;
			tok = m_Parser.NextToken();
		}
	}

	void Abort()
	{
		m_Break = true;
	}

	Parser* GetParser()
	{
		return &m_Parser;
	}

private:
	// For any X-Files
	Parser m_Parser;
	ErrorReporter* m_ErrRep;

	core::RawMemory m_Buffer;
	XFileVisitor* m_Visitor;
	bool m_Break;
};

class BaseVisitor : public XFileVisitor
{
public:
	BaseVisitor(XFileTraverser* traverser) :
		XFileVisitor(traverser, traverser->GetParser())
	{
	}

	void EnterScope(const Scope& scope)
	{
		m_Scope.EmplaceBack(scope);

		if(scope.type == g_GUIDFrame)
			m_MatrixStack.Push(math::Matrix4::IDENTITY);
	}

	bool OnObject(const Scope& scope)
	{
		LUX_UNUSED(scope);
		return false;
	}

	void LeaveScope(const Scope& scope)
	{
		if(scope.type == g_GUIDFrame)
			m_MatrixStack.Pop();

		m_Scope.PopBack();
	}

	bool InScope(const core::GUID& parentType, int maxDistance = 1)
	{
		int count = math::Min(maxDistance + 1, m_Scope.Size());
		for(int i = 1; i < count; ++i) {
			if(m_Scope.Back(i).type == parentType)
				return true;
		}
		return false;
	}

protected:
	math::MatrixStack m_MatrixStack;
	core::Array<Scope> m_Scope;
};

class MeshLoadVisitor : public BaseVisitor
{
	struct XMaterial;
	struct XMeshData;
	static const u32 INVALID_BUFFERID = 0xFFFFFFFF;
	static const u32 INVALID_CORNERID = 0xFFFFFFFF;

public:
	MeshLoadVisitor(Mesh* dst, XFileTraverser* traverser, const io::Path& baseDir, const core::String& meshToLoad, bool merge) :
		BaseVisitor(traverser),
		m_BaseDir(baseDir)
	{
		m_Mesh.ResetFull(dst);
		m_MeshToLoad = meshToLoad;
		m_MergeMeshes = merge;
	}

	bool OnObject(const Scope& scope)
	{
		if(scope.type == g_GUIDFrame) {
			/*
			[...]
			*/
			traverser->ReadOpenObjectData();
		} else if(scope.type == g_GUIDFrameTransformMatrix) {
			/*
			FrameTransformMatrix:
				Matrix4x4 frameMatrix;
			Matrix4x4:
				array float values[16];
			*/
			if(!InScope(g_GUIDFrame))
				throw XFileFormatException("FrameTransformMatrix only inside frame");
			math::Matrix4 m;
			parser->ReadFloatArray(16, m.DataRowMajor());
			// Replace default identity matrix.
			m_MatrixStack.Pop();
			m_MatrixStack.Push(m);
			parser->MatchTextOnly(TOKEN_SEMICOLON);
		} else if(scope.type == g_GUIDMesh) {
			/*
			Mesh:
				DWORD nVertices;
				array Vector vertices[nVertices];
				DWORD nFaces;
				array MeshFace faces[nFaces];
				[...]
			*/
			if(!InScope(g_GUIDFrame))
				throw XFileFormatException("Meshes only inside frame");

			if(!m_MergeMeshes && scope.identName != m_MeshToLoad)
				return false;

			u32 nVertices = parser->ReadDWORD();
			m_Mesh.positions.Resize(nVertices);

			parser->ReadArray(nVertices, [&](int i) {
				parser->ReadFloatElements(3, &m_Mesh.positions[i]);
			});

			u32 nFaces = parser->ReadDWORD();
			m_Mesh.faces.Clear();
			m_Mesh.faces.Reserve(nFaces);
			m_Mesh.corners.Reserve(nFaces * 3); // Just an estimation
			m_Mesh.vertex2Corner.Resize(nVertices);
			memset(m_Mesh.vertex2Corner.Data(), 0xFF, nVertices * sizeof(m_Mesh.vertex2Corner[0]));

			u32 cornerId = 0;
			parser->ReadArray(nFaces, [&](u32) {
				u32 count = parser->ReadDWORD();
				if(count < 3)
					throw XFileFormatException("Face must have at least 3 corners");
				auto ptr = parser->ReadDWORDArray(count);
				m_Mesh.triangleCount += count - 2;
				m_Mesh.faces.PushBack({cornerId, count, 0});
				for(u32 j = 0; j < count; ++j) {
					u32 vid = ptr[j];
					auto curCorner = m_Mesh.vertex2Corner[vid];
					auto prevCorner = INVALID_CORNERID;
					if(curCorner != INVALID_CORNERID) {
						m_Mesh.vertex2Corner[vid] = curCorner;
						prevCorner = m_Mesh.corners[curCorner].ref;
						m_Mesh.corners[curCorner].ref = cornerId;
					} else {
						m_Mesh.vertex2Corner[vid] = cornerId;
					}
					m_Mesh.corners.PushBack({vid, INVALID_CORNERID, prevCorner, INVALID_BUFFERID});
					++cornerId;
				}
			});
			traverser->ReadOpenObjectData();
		} else if(scope.type == g_GUIDMeshNormals) {
			/*
			MeshNormals:
				DWORD nNormals;
				array Vector normals[nNormals];
				DWORD nFaceNormals;
				array MeshFace faceNormals[nFaceNormals];
			*/
			if(!InScope(g_GUIDMesh))
				throw XFileFormatException("Normals only inside mesh");

			u32 nNormals = parser->ReadDWORD();
			m_Mesh.normals.Resize(nNormals);
			parser->ReadArray(nNormals, [&](u32 i) {
				parser->ReadFloatElements(3, &m_Mesh.normals[i]);
			});

			u32 nFaceNormals = parser->ReadDWORD();
			int cornerId = 0;
			parser->ReadArray(nFaceNormals, [&](u32) {
				u32 count = parser->ReadDWORD();
				auto ptr = parser->ReadDWORDArray(count);
				for(u32 j = 0; j < count; ++j) {
					m_Mesh.corners[cornerId].nid = ptr[j];
					++cornerId;
				}
				if(cornerId > std::numeric_limits<int>::max())
					throw XFileFormatException("Too many vertices");
			});
		} else if(scope.type == g_GUIDMeshTextureCoords) {
			/*
			MeshTextureCoords:
				DWORD nTextureCoords;
				array Coords2d textureCoords[nTextureCoords];
			*/
			if(!InScope(g_GUIDMesh))
				throw XFileFormatException("Texturecoords only inside mesh");

			u32 nTextureCoords = parser->ReadDWORD();
			if(nTextureCoords != (u32)m_Mesh.positions.Size())
				throw XFileFormatException("Texturecoords count != vertex count");
			m_Mesh.texCoords.Resize(nTextureCoords);
			parser->ReadArray(nTextureCoords, [&](u32 i) {
				parser->ReadFloatElements(2, &m_Mesh.texCoords[i]);
			});
		} else if(scope.type == g_GUIDMeshMaterialList) {
			/*
			DWORD nMaterials; # Total number of materials
			DWORD nFaceIndexes; # Number of faces
			array DWORD faceIndexes[nFaceIndexes]; # The material used for each face
			[Material <3D82AB4D-62DA-11CF-AB39-0020AF71E433>]
			*/
			if(!InScope(g_GUIDMesh))
				throw XFileFormatException("Materiallist only inside mesh");
			auto matCount = parser->ReadDWORD();
			m_Mesh.materials.Resize(matCount);
			auto faceCount = parser->ReadDWORD();
			if(faceCount != (u32)m_Mesh.faces.Size())
				throw XFileFormatException("Material face count != Mesh face count");
			auto faceIndexes = parser->ReadDWORDArray(faceCount);
			for(u32 i = 0; i < faceCount; ++i) {
				m_Mesh.faces[i].material = faceIndexes[i];
				m_Mesh.materials[faceIndexes[i]].triangleCount += m_Mesh.faces[i].corners - 2;
			}
			traverser->ReadOpenObjectData(XFileTraverser::RESTRICTED, g_GUIDMaterial);
		} else if(scope.type == g_GUIDMaterial) {
			/*
			ColorRGBA faceColor;
			FLOAT power;
			ColorRGB specularColor;
			ColorRGB emissiveColor;
			[...]
			*/
			if(!InScope(g_GUIDMeshMaterialList))
				throw XFileFormatException("Material only inside Materiallist");

			if(m_Mesh.nextMaterialId >= (u32)m_Mesh.materials.Size()) {
				Warning("Ignoring unreferenced materials");
				return false;
			}
			auto& material = m_Mesh.materials[m_Mesh.nextMaterialId++];
			float color[4];

			// Ignore the alpha, it seems to not mean alpha.
			parser->ReadFloatElements(4, color);
			parser->MatchTextOnly(TOKEN_SEMICOLON);
			material.faceColor.Set(1, color[0], color[1], color[2]);

			parser->ReadFloatElements(1, &material.power);

			parser->ReadFloatElements(3, color);
			parser->MatchTextOnly(TOKEN_SEMICOLON);
			material.specularColor.Set(1, color[0], color[1], color[2]);

			parser->ReadFloatElements(3, color);
			parser->MatchTextOnly(TOKEN_SEMICOLON);
			material.emmisiveColor.Set(color[3], color[0], color[1], color[2]);

			traverser->ReadOpenObjectData();
		} else if(scope.type == g_GUIDTextureFileName) {
			/*
			string filename;
			*/
			if(!InScope(g_GUIDMaterial))
				throw XFileFormatException("TextureFileName only inside Material");

			auto& material = m_Mesh.materials.Back();
			auto string = parser->ReadString();
			material.textures.PushBack(string);
		} else if(scope.type == g_GUIDSkinWeights) {
			/*
			STRING transformNodeName;
			DWORD nWeights;
			array DWORD vertexIndices[nWeights];
			array float weights[nWeights];
			Matrix4x4 matrixOffset;
			*/
			if(!InScope(g_GUIDMesh))
				throw XFileFormatException("MeshSkinWeights only inside Mesh");

			auto boneName = parser->ReadString();
			int boneId = -1;
			for(int i = 0; i < m_Mesh.boneNames.Size(); ++i) {
				if(m_Mesh.boneNames[i] == boneName) {
					boneId = i;
					break;
				}
			}
			if(boneId == -1) {
				boneId = m_Mesh.boneNames.Size();
				m_Mesh.boneNames.PushBack(boneName);
			}

			if(m_Mesh.boneIndices.IsEmpty()) {
				m_Mesh.boneIndices.Resize(m_Mesh.positions.Size());
				m_Mesh.boneWeights.Resize(m_Mesh.positions.Size());
				m_Mesh.vertexBoneAffectCount.Resize(m_Mesh.positions.Size());
				memset(m_Mesh.boneIndices.Data(), 0, m_Mesh.boneIndices.Size() * 4);
				memset(m_Mesh.boneWeights.Data(), 0, m_Mesh.boneWeights.Size() * 4);
				memset(m_Mesh.vertexBoneAffectCount.Data(), 0, m_Mesh.vertexBoneAffectCount.Size());
			}

			auto nWeights = parser->ReadDWORD();
			buffer.Resize(nWeights * 4);
			auto vertexIndices = parser->ReadDWORDArray(nWeights, buffer.Data());
			const int MAX_BONES_PER_VERTEX = 4;
			for(u32 i = 0; i < nWeights; ++i) {
				auto vid = vertexIndices[i];
				auto id = m_Mesh.vertexBoneAffectCount[vid];
				m_Mesh.boneIndices[vid][id] = (u8)boneId;
			}

			auto weights = parser->ReadFloatArray(nWeights, nullptr);
			int i = 0;
			for(auto f : core::MakeRange(weights, weights + nWeights)) {
				auto vid = vertexIndices[i];
				auto& id = m_Mesh.vertexBoneAffectCount[vid];
				if(id == MAX_BONES_PER_VERTEX) {
					u8 minValue = (u8)(f * 255);
					int min = 0;
					for(; min < MAX_BONES_PER_VERTEX; ++min) {
						if(m_Mesh.boneWeights[vid][min] < minValue) {
							minValue = m_Mesh.boneWeights[vid][min];
							break;
						}
					}
					if(min < MAX_BONES_PER_VERTEX) {
						m_Mesh.boneWeights[vid][min] = (u8)(f * 255);
						m_Mesh.boneIndices[vid][min] = m_Mesh.boneIndices[vid][id];
					}
				} else {
					m_Mesh.boneWeights[vid][id] = (u8)(f * 255);
					++id;
				}
				++i;
			}

			math::Matrix4 matrixOffset;
			parser->ReadFloatArray(16, matrixOffset.DataRowMajor());
			parser->MatchTextOnly(TOKEN_SEMICOLON);
		} else {
			return false;
		}
		return true;
	}

	void LeaveScope(const Scope& scope)
	{
		if(scope.type == g_GUIDMesh) {
			if(!m_MergeMeshes && scope.identName != m_MeshToLoad)
				return;
			// Create final mesh.
			StrongRef<video::Geometry> geo = m_Mesh.dst->GetGeometry();
			if(!geo) {
				geo = video::VideoDriver::Instance()->CreateGeometry(
					m_Mesh.boneIndices.Size() ? g_VertexFormatSkinned : video::VertexFormat::STANDARD,
					video::EPrimitiveType::Triangles);
				geo->GetIndices()->SetFormat(video::EIndexFormat::Bit16);
			}
			auto vb = geo->GetVertices();
			auto ib = geo->GetIndices();
			vb->Reserve(m_Mesh.vertexOffset + m_Mesh.positions.Size());

			int indexStride = 2;
			int vertexStride = vb->GetStride();
			void* iBaseData;
			core::Array<void*> materialGroupPointer;
			if(!m_Mesh.materials.IsEmpty()) {
				int totalCount = 0;
				for(auto& m : m_Mesh.materials)
					totalCount += m.triangleCount;
				ib->SetSize(m_Mesh.indexOffset + totalCount * 3);
				iBaseData = ib->Pointer();
				auto iData = (u8*)iBaseData + m_Mesh.indexOffset * indexStride;
				for(auto& m : m_Mesh.materials) {
					materialGroupPointer.PushBack(iData);
					iData = (u8*)iData + indexStride * m.triangleCount * 3;
				}
			} else {
				ib->SetSize(m_Mesh.indexOffset + m_Mesh.triangleCount * 3);
				iBaseData = (u8*)ib->Pointer() + m_Mesh.indexOffset * indexStride;
				materialGroupPointer.PushBack(iBaseData);
			}

			auto getBufferId = [&](u32 vid, u32 nid) {
				struct
				{
					video::Vertex3D vertex;
					u8 indices[4];
					u8 weights[4];
				} vertex;
				auto corner = m_Mesh.vertex2Corner[vid];
				lxAssert(corner != INVALID_CORNERID);
				u32 bufferId = INVALID_BUFFERID;
				while(corner != INVALID_BUFFERID) {
					if(m_Mesh.corners[corner].nid == nid) {
						bufferId = m_Mesh.corners[corner].bufferid;
						break;
					}
					corner = m_Mesh.corners[corner].ref;
					lxAssert(corner != INVALID_CORNERID);
				}

				if(bufferId == INVALID_BUFFERID) {
					vertex.vertex.position = m_Mesh.positions[vid];
					if(!m_Mesh.texCoords.IsEmpty())
						vertex.vertex.texture = m_Mesh.texCoords[vid];
					vertex.vertex.normal = m_Mesh.normals[nid];
					if(m_Mesh.boneIndices.Size()) {
						memcpy(vertex.indices, &m_Mesh.boneIndices[vid], 4);
						memcpy(vertex.weights, &m_Mesh.boneWeights[vid], 4);
					}
					bufferId = vb->AddVertex(&vertex);
					m_Mesh.corners[corner].bufferid = bufferId;
				}
				lxAssert(bufferId != INVALID_BUFFERID);
				if(bufferId > (u32)std::numeric_limits<int>::max())
					throw XFileFormatException("Too many vertices");
				return bufferId;
			};

			u32 indexId = 0;
			for(auto& face : m_Mesh.faces) {
				auto corners = &m_Mesh.corners[face.begin];
				auto firstId = getBufferId(corners[0].vid, corners[0].nid);
				auto lastId = getBufferId(corners[1].vid, corners[1].nid);
				auto& cur = materialGroupPointer[face.material];
				for(auto corner : core::MakeRange(corners + 2, corners + face.corners)) {
					*(u16*)cur = (u16)firstId;
					cur = (u16*)cur + 1;
					*(u16*)cur = (u16)lastId;
					cur = (u16*)cur + 1;
					*(u16*)cur = (u16)(lastId = getBufferId(corner.vid, corner.nid));
					cur = (u16*)cur + 1;
					indexId += 3;
				}
				if(indexId > (u32)std::numeric_limits<int>::max())
					throw XFileFormatException("Too many vertices");
			}

			if(!m_MatrixStack.PeekAbs().IsIdent()) {
				auto nVertices = vb->GetSize() - m_Mesh.vertexOffset;
				auto& mat = m_MatrixStack.PeekAbs();
				auto matIT = mat.GetTransformInverted().GetTransposed();
				auto vdata = (u8*)vb->Pointer() + vertexStride * m_Mesh.vertexOffset;
				for(u32 i = 0; i < nVertices; ++i) {
					auto& vertex = *(video::Vertex3D*)(vdata + i*vertexStride);
					vertex.position = mat.TransformVector(vertex.position);
					vertex.normal = matIT.TransformVector(vertex.normal);
				}
			}

			m_Mesh.dst->SetGeometry(geo);

			int lastId = 0;
			for(int i = 0; i < m_Mesh.materials.Size(); ++i) {
				auto lxMat = MakeLuxMaterial(m_Mesh.materials[i]);
				auto nextId = ((u8*)materialGroupPointer[i] - (u8*)iBaseData) / (3 * indexStride) - 1;
				m_Mesh.dst->SetMaterialRange(lxMat, lastId + m_Mesh.indexOffset / 3, nextId + m_Mesh.indexOffset / 3);
				lastId = nextId + 1;
			}

			// Write Skinning extradata
			if(m_Mesh.boneNames.Size()) {
				StrongRef<video::MeshExDataBoneTable> bonetable = LUX_NEW(video::MeshExDataBoneTable);
				bonetable->boneNames = m_Mesh.boneNames;
				bonetable->maxSkinWeightPerVertex = 4;
				m_Mesh.dst->AddExData(bonetable);
			}

			if(m_MergeMeshes) {
				m_Mesh.Reset();
				m_Mesh.indexOffset = ib->GetSize();
				m_Mesh.vertexOffset = vb->GetSize();
			} else {
				traverser->Abort();
			}
		}

		BaseVisitor::LeaveScope(scope);
	}

	StrongRef<io::File> FindFile(const io::Path& path)
	{
		auto fileSys = io::FileSystem::Instance();

		auto relFile = path.GetResolved(m_BaseDir);
		if(fileSys->ExistFile(relFile))
			return fileSys->OpenFile(relFile);
		if(fileSys->ExistFile(path))
			return fileSys->OpenFile(path);
		return nullptr;
	}

	StrongRef<video::Material> MakeLuxMaterial(const XMaterial& xmat)
	{
		video::MaterialLibrary::EKnownMaterial matType;
		if(xmat.faceColor.a != 1.0f)
			matType = video::MaterialLibrary::EKnownMaterial::Transparent;
		else
			matType = video::MaterialLibrary::EKnownMaterial::Solid;

		auto material = video::MaterialLibrary::Instance()->CloneMaterial(matType);
		StrongRef<video::Texture> tex;
		if(xmat.textures.Size() > 1)
			Warning("DirectX Mesh Loader: Can only load single texture layer: Others are ignored.");
		if(xmat.textures.Size() > 0) {
			auto texFile = FindFile(xmat.textures[0]);
			tex = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Texture, texFile).StaticCastStrong<video::Texture>();
		}
		material->SetDiffuse(xmat.faceColor);
		material->SetSpecularHardness(xmat.power);
		material->SetSpecularIntensity(xmat.specularColor.GetLuminance());
		material->SetEmissive(xmat.emmisiveColor.GetLuminance());
		material->SetTexture(0, tex);
		auto p = material->GetPass();
		p.culling = video::EFaceSide::None;
		material->SetPass(p);
		return material;
	}

	void Warning(core::StringView warning)
	{
		log::Warning(warning);
	}

private:
	struct XMaterial
	{
		video::ColorF faceColor;
		float power;
		video::ColorF specularColor;
		video::ColorF emmisiveColor;

		core::Array<core::String> textures;

		int triangleCount = 0;
	};

	struct ByteQuadruple
	{
		u8 data[4];
		u8 operator[](int i) const { return data[i]; }
		u8& operator[](int i) { return data[i]; }
	};
	struct XMeshData
	{
		video::Mesh* dst;

		core::Array<core::String> boneNames;

		// Vertex elements
		core::Array<u8> vertexBoneAffectCount;
		core::Array<ByteQuadruple> boneWeights;
		core::Array<ByteQuadruple> boneIndices;

		core::Array<math::Vector3F> positions;
		core::Array<math::Vector2F> texCoords;
		core::Array<video::Color> colors;

		// The index of the first corner belonging to the vertex
		core::Array<u32> vertex2Corner;

		// Per face data
		core::Array<math::Vector3F> normals;
		struct Corner
		{
			u32 vid;
			u32 nid;

			u32 ref; // The next corner which shares the same vertex as this corner
			u32 bufferid; // Id of the vertex belonging to this corner.
		};
		core::Array<Corner> corners;
		struct FaceInfo
		{
			u32 begin; // The first corner of this face.
			u32 corners; // The number of corners of this face
			u32 material;
		};
		core::Array<FaceInfo> faces;
		u32 triangleCount;

		u32 nextMaterialId;
		core::Array<XMaterial> materials;

		u32 vertexOffset;
		u32 indexOffset;

		void ResetFull(video::Mesh* mesh)
		{
			Reset();
			dst = mesh;
			vertexOffset = 0;
			indexOffset = 0;
		}

		void Reset()
		{
			positions.Clear();
			texCoords.Clear();
			colors.Clear();

			vertex2Corner.Clear();

			vertexBoneAffectCount.Clear();
			boneWeights.Clear();
			boneIndices.Clear();

			normals.Clear();
			corners.Clear();
			faces.Clear();
			triangleCount = 0;

			nextMaterialId = 0;
			materials.Clear();
		}
	};

	core::Array<u8> buffer;
	XMeshData m_Mesh;
	io::Path m_BaseDir;
	bool m_MergeMeshes = true;
	core::String m_MeshToLoad;
};

} //namespace xfile_loader_impl

MeshLoaderX::MeshLoaderX()
{
	using namespace xfile_loader_impl;

	g_GUIDFrame = core::GUID::FromString("3D82AB46-62DA-11CF-AB39-0020AF71E433");
	g_GUIDFrameTransformMatrix = core::GUID::FromString("F6F23F41-7686-11cf-8F52-0040333594A3");
	g_GUIDMesh = core::GUID::FromString("3D82AB44-62DA-11CF-AB39-0020AF71E433");
	g_GUIDMeshNormals = core::GUID::FromString("F6F23F43-7686-11cf-8F52-0040333594A3");
	g_GUIDMeshTextureCoords = core::GUID::FromString("F6F23F40-7686-11cf-8F52-0040333594A3");
	g_GUIDTextureFileName = core::GUID::FromString("A42790E1-7810-11cf-8F52-0040333594A3");
	g_GUIDMaterial = core::GUID::FromString("3D82AB4D-62DA-11CF-AB39-0020AF71E433");
	g_GUIDMeshMaterialList = core::GUID::FromString("F6F23F42-7686-11CF-8F52-0040333594A3");
	g_GUIDSkinWeights = core::GUID::FromString("6F0D123B-BAD2-4167-A0D0-80224F25FABB");
	g_TypeGUIDS["MESH"] = g_GUIDMesh;
	g_TypeGUIDS["FRAME"] = g_GUIDFrame;
	g_TypeGUIDS["FRAMETRANSFORMMATRIX"] = g_GUIDFrameTransformMatrix;
	g_TypeGUIDS["MESHNORMALS"] = g_GUIDMeshNormals;
	g_TypeGUIDS["MESHTEXTURECOORDS"] = g_GUIDMeshTextureCoords;
	g_TypeGUIDS["TEXTUREFILENAME"] = g_GUIDTextureFileName;
	g_TypeGUIDS["MATERIAL"] = g_GUIDMaterial;
	g_TypeGUIDS["MESHMATERIALLIST"] = g_GUIDMeshMaterialList;
	g_TypeGUIDS["SKINWEIGHTS"] = g_GUIDSkinWeights;

	g_KeywordMap["ARRAY"] = TOKEN_ARRAY;
	g_KeywordMap["BINARY"] = TOKEN_NAME;
	g_KeywordMap["BINARY_RESOURCE"] = TOKEN_NAME;
	g_KeywordMap["CHAR"] = TOKEN_CHAR;
	g_KeywordMap["CSTRING"] = TOKEN_CSTRING;
	g_KeywordMap["DOUBLE"] = TOKEN_DOUBLE;
	g_KeywordMap["DWORD"] = TOKEN_DWORD;
	g_KeywordMap["FLOAT"] = TOKEN_FLOAT;
	g_KeywordMap["SDWORD"] = TOKEN_SDWORD;
	g_KeywordMap["STRING"] = TOKEN_STRING;
	g_KeywordMap["SWORD"] = TOKEN_SWORD;
	g_KeywordMap["TEMPLATE"] = TOKEN_TEMPLATE;
	g_KeywordMap["UCHAR"] = TOKEN_UCHAR;
	g_KeywordMap["ULONGLONG"] = TOKEN_INVALID;
	g_KeywordMap["UNICODE"] = TOKEN_UNICODE;
	g_KeywordMap["WORD"] = TOKEN_WORD;

	video::VertexDeclaration decl;
	decl.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
	decl.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
	decl.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
	decl.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
	decl.AddElement(VertexElement::EUsage::BlendIndices, VertexElement::EType::Byte4);
	decl.AddElement(VertexElement::EUsage::BlendWeight, VertexElement::EType::Byte4);
	g_VertexFormatSkinned = VertexFormat("BoneFormat", decl);
}

core::Name MeshLoaderX::GetResourceType(io::File* file, core::Name requestedType)
{
	bool isSupportedType =
		requestedType == core::ResourceType::Mesh || requestedType == "armature";
	if(!requestedType.IsEmpty() && !isSupportedType)
		return core::Name::INVALID;

	if(requestedType.IsEmpty())
		requestedType = core::ResourceType::Mesh;

	u8 magic[4];
	if(file->ReadBinaryPart(4, magic) != 4)
		return core::Name::INVALID;

	if(memcmp(magic, "xof ", 4) != 0)
		return core::Name::INVALID;
	else
		return requestedType;
}

void MeshLoaderX::LoadResource(io::File* file, core::Resource* dst)
{
	auto mesh = dynamic_cast<video::Mesh*>(dst);
	if(mesh)
		LoadMesh(file, mesh);
}

void MeshLoaderX::LoadMesh(io::File* file, video::Mesh* dst, const core::String& meshToLoad, bool mergeMeshes)
{
	using namespace xfile_loader_impl;
	XFileTraverser traverser;
	ErrorReporter errReporter;

	dst->SetGeometry(nullptr);
	MeshLoadVisitor meshLoader(dst, &traverser, file->GetPath(), meshToLoad, mergeMeshes);
	traverser.Traverse(&meshLoader, file, &errReporter);

	if(dst->GetMaterialCount() == 0) {
		dst->SetMaterial(
			video::MaterialLibrary::Instance()->GetMaterial(
				video::MaterialLibrary::EKnownMaterial::Solid));
	}
}

const core::String& MeshLoaderX::GetName() const
{
	return xfile_loader_impl::g_MeshLoaderName;
}

} // namespace video
} // namespace lux