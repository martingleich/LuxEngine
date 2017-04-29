#ifndef INCLUDED_CSCENEVALUES_H
#define INCLUDED_CSCENEVALUES_H
#include "video/SceneValues.h"
#include "core/lxArray.h"

namespace lux
{
namespace scene
{

class SceneValuesImpl : public SceneValues
{
public:
	// Eine Liste aller Parameter
	struct SParam
	{
		string name;
		u32 offset;
		core::Type type;
		u32 Size;

		SParam()
		{
		}

		SParam(const string& _Name, u32 _Offset, core::Type _Type, u32 _Size) : name(_Name), offset(_Offset), type(_Type), Size(_Size)
		{
		}

		SParam& operator=(const SParam& other)
		{
			name = other.name;
			offset = other.offset;
			type = other.type;
			Size = other.Size;

			return *this;
		}
	};

private:
	static const u32 MATRIX_COUNT = 16;
	static_assert(MATRIX_COUNT <= sizeof(u32)*8, "Too many matrizes.");

	static const u32 FIRST_LIGHT_ID = 10000;

	struct LightMatrix
	{
		math::matrix4 matrix;
		string name;

		LightMatrix()
		{
			matrix(0, 3) = 0.0f;
		}
	};

private:
	core::array<LightMatrix> m_Lights;
	math::matrix4 m_InvalidLight;

	core::array<SParam> m_Params;
	u8* m_ParamData;
	u32 m_CurrentOffset;
	u32 m_MaxSize;

	// Matrizen als seperaten Haufen speichern
	/*
	Basismatrizen:
	World = 1
	View = 2
	Proj = 4

	WorldI = 8
	ViewI = 16

	// Kombinierte Matrizen
	WorldView = 32
	WorldProj = 64
	ViewProj = 128
	WorldViewProj = 256

	WorldViewI = 512

	WorldT = 1024
	ViewT = 2048
	WorldIT = 4096
	ViewIT = 8192
	WorldViewT = 16384
	WorldViewIT = 32768

	// Modifizierte Matrizen
	T zum Transpondieren
	WorldIT = InvertierteTranspondierte Weltmatrix
	Matrizen die eine Projektion ernthalten sind nicht invertierbar

	Macht ingesamt 10 Matrizen = 640 Byte
	*/
	mutable math::matrix4 m_Matrizes[MATRIX_COUNT];
	mutable u32 m_UpdateMatrizes;    // 0 wenn die Matrix nicht aktualisiert werden muss 1 falls doch

	static const string MATRIX_NAMES[MATRIX_COUNT];

private:
	void Realloc(u32 newSize);

	bool IsUpToDate(EMatrizes type) const;

public:
	SceneValuesImpl(bool AllMatrizesToIdent = true);
	~SceneValuesImpl();

	void SetMatrix(EMatrizes type, const math::matrix4& Matrix);
	void SetMatrix(EMatrizes type, const math::matrix4& Matrix, const math::matrix4& InvMatrix);

	const math::matrix4& GetMatrix(EMatrizes type) const;

	u32 AddParam(const string& name, core::Type type);

	bool SetLight(u32 id, const video::LightData& light);
	void ClearLights();

	/*
	void RemoveParam(u32 id);
	void RemoveAllParams();
	*/
	u32 GetParamCount() const;

	u32 GetParamID(const string& name) const;

	const string& GetParamName(u32 id) const;

	core::Type GetParamType(u32 id) const;

	const void* GetParamValue(u32 id) const;

	void SetParamValue(u32 id, const void* p);

	u32 GetSysMatrixCount() const;
};

}
}

#endif // !INCLUDED_CSCENEVALUES_H
