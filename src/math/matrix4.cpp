#include "math/matrix4.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Matrix()
{
	static const Type t(new core::TypeInfoTemplate<math::Matrix4>("matrix4"));
	return t;
}
}
}

namespace math
{

const Matrix4 Matrix4::IDENTITY = math::Matrix4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);
const Matrix4 Matrix4::ZERO = math::Matrix4(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f);

//******************************************************************
// Konstruktor
Matrix4::Matrix4(EMatrix4Constructor Constructor)
{
	switch(Constructor) {
	case M4C_NOTHING:
	case M4C_COPY:
		break;
	default:
		MakeIdent();
	};
}

//******************************************************************
// Kopierkonstruktor
Matrix4::Matrix4(const Matrix4& other, EMatrix4Constructor Constructor)
{
	switch(Constructor) {
	case M4C_NOTHING:
		break;
	case M4C_IDENT:
		MakeIdent();
		break;
	case M4C_COPY:
		*this = other;
		break;
	case M4C_INV:
		if(!other.GetInverted(*this)) {
			// Bei Fehler 0-Matrix
			MakeIdent();
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 0.0f;
		}
		break;
	case M4C_TRANSP:
		other.GetTransposed(*this);
		break;
	case M4C_INV_TRANSP:
		if(!other.GetInverted(*this)) {
			// Bei Fehler 0-Matrix
			MakeIdent();
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 0.0f;
		} else
			this->Transpose();
		break;
	};
}

//******************************************************************
// Prüft ob die Matrix die Identitätsmatrix ist
bool Matrix4::IsIdent() const
{
	// Zuerst die letzte Zeile überprüfen, da Matrizen meist mindesten Translationen enthalten
	if(!IsZero(m[3][0]) || !IsZero(m[3][1]) || !IsZero(m[3][2]) || !IsEqual(m[3][3], 1.0f))
		return false;

	if(!IsEqual(m[0][0], 1.0f) || !IsZero(m[0][1]) || !IsZero(m[0][2]) || !IsZero(m[0][3]))
		return false;

	if(!IsZero(m[1][0]) || !IsEqual(m[1][1], 1.0f) || !IsZero(m[1][2]) || !IsZero(m[1][3]))
		return false;

	if(!IsZero(m[2][0]) || !IsZero(m[2][1]) || !IsEqual(m[2][2], 1.0f) || !IsZero(m[2][3]))
		return false;

	return true;
}

//******************************************************************
// Macht die Matrix zur Identiätsmatrix
Matrix4& Matrix4::MakeIdent()
{
	*this = IDENTITY;

	return *this;
}

//******************************************************************
//Wandelt eine 4x4 Matrix in eine 3x3 Matrix um,zur Transformation von Texturkoordinaten
void Matrix4::Get3x3(Matrix4& out) const
{
	out = Matrix4(m[0][0], m[0][1], m[0][3], 0.0f,
		m[1][0], m[1][1], m[1][3], 0.0f,
		m[3][0], m[3][1], m[3][3], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

//******************************************************************
//Gibt die Determinante der Matrix aus
float Matrix4::GetDet() const
{
	//Die linke obere 3x3 Matrix reicht für unsere Zwecke aus
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

//******************************************************************
// Transpondiert diese Matrix
Matrix4& Matrix4::Transpose()
{
	(*this) = Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
	return *this;
}

//******************************************************************
// Gib die transpondierte Matrix aus
void Matrix4::GetTransposed(Matrix4& out) const
{
	out = Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
}

//******************************************************************
// Invertiert diese Matrix
// Bei Fehler keine Auswirkung
Matrix4& Matrix4::Invert()
{
	//Kehrwert der Determinante vorberechnen
	float fInvDet = GetDet();
	if(fInvDet == 0) return *this;
	fInvDet = 1.0f / fInvDet;

	// Invertierte Matrix berechnen
	Matrix4 Temp(M4C_NOTHING);
	this->GetInverted(Temp);

	*this = Temp;
	return *this;
}

//******************************************************************
// Liefert die Invertierte Matrix
// False wenn sich Matrix nicht invertieren lässt
bool Matrix4::GetInverted(Matrix4& out) const
{
	//Kehrwert der Determinante vorberechnen
	float fInvDet = GetDet();
	if(IsZero(fInvDet)) return false;
	fInvDet = 1.0f / fInvDet;

	// Invertierte Matrix berechnen
	out.m[0][0] = fInvDet * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
	out.m[0][1] = -fInvDet * (m[0][1] * m[2][2] - m[0][2] * m[2][1]);
	out.m[0][2] = fInvDet * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
	out.m[0][3] = 0.0f;
	out.m[1][0] = -fInvDet * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
	out.m[1][1] = fInvDet * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
	out.m[1][2] = -fInvDet * (m[0][0] * m[1][2] - m[0][2] * m[1][0]);
	out.m[1][3] = 0.0f;
	out.m[2][0] = fInvDet * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	out.m[2][1] = -fInvDet * (m[0][0] * m[2][1] - m[0][1] * m[2][0]);
	out.m[2][2] = fInvDet * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
	out.m[2][3] = 0.0f;
	out.m[3][0] = -(m[3][0] * out.m[0][0] + m[3][1] * out.m[1][0] + m[3][2] * out.m[2][0]);
	out.m[3][1] = -(m[3][0] * out.m[0][1] + m[3][1] * out.m[1][1] + m[3][2] * out.m[2][1]);
	out.m[3][2] = -(m[3][0] * out.m[0][2] + m[3][1] * out.m[1][2] + m[3][2] * out.m[2][2]);
	out.m[3][3] = 1.0f;

	return true;
}

//******************************************************************
// Setzt die Verschiebung dieser Matrix
Matrix4& Matrix4::SetTranslation(const Vector3F& vTrans)
{
	m[3][0] = vTrans.x; m[3][1] = vTrans.y; m[3][2] = vTrans.z;
	return *this;
}

//******************************************************************
// Adds a Translation to this Matrix
Matrix4& Matrix4::AddTranslation(const Vector3F& vTrans)
{
	m[0][0] += m[0][3] * vTrans.x;    m[0][1] += m[0][3] * vTrans.y;    m[0][2] += m[0][3] * vTrans.z;
	m[1][0] += m[1][3] * vTrans.x;    m[1][1] += m[1][3] * vTrans.y;    m[1][2] += m[1][3] * vTrans.z;
	m[2][0] += m[2][3] * vTrans.x;    m[2][1] += m[2][3] * vTrans.y;    m[2][2] += m[2][3] * vTrans.z;
	m[3][0] += m[3][3] * vTrans.x;    m[3][1] += m[3][3] * vTrans.y;    m[3][2] += m[3][3] * vTrans.z;

	return *this;
}

//******************************************************************
// Fragt die Verschiebung durch diese Matrix ab
Vector3F Matrix4::GetTranslation() const
{
	return Vector3F(m[3][0], m[3][1], m[3][2]);
}

//******************************************************************
// Setzt die Skalierung dieser Matrix
// Sollte nicht benutzt werden, wenn bereits eine Rotation in der Matrix enthalten ist
// in diesem Fall per Matrixmultiplikation hinzufügen
Matrix4& Matrix4::SetScale(const Vector3F& vScale)
{
	m[0][0] = vScale.x;
	m[1][1] = vScale.y;
	m[2][2] = vScale.z;
	return *this;
}

Matrix4& Matrix4::AddScale(const Vector3F& vScale)
{
	m[0][0] *= vScale.x;    m[0][1] *= vScale.y;    m[0][2] *= vScale.z;
	m[1][0] *= vScale.x;    m[1][1] *= vScale.y;    m[1][2] *= vScale.z;
	m[2][0] *= vScale.x;    m[2][1] *= vScale.y;    m[2][2] *= vScale.z;
	m[3][0] *= vScale.x;    m[3][1] *= vScale.y;    m[3][2] *= vScale.z;

	return *this;
}

//******************************************************************
// Fragt die Skalierung durch diese Matrix ab
Vector3F Matrix4::GetScale() const
{
	// 0 Rotationsfall abfangen
	if(IsZero(m[0][1]) && IsZero(m[0][2]) &&
		IsZero(m[1][0]) && IsZero(m[1][2]) &&
		IsZero(m[2][0]) && IsZero(m[2][1]))
		return Vector3F(m[0][0], m[1][1], m[2][2]);

	// Volle Berechnung nötig
	float XScaling = sqrtf(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]);
	return Vector3F(GetDet() > 0 ? XScaling : -XScaling,
		sqrtf(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]),
		sqrtf(m[2][0] * m[0][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]));
}

//******************************************************************
// Setzt die Rotation dieser Matrix
Matrix4& Matrix4::SetRotationEuler(AngleF x, AngleF y, AngleF z)
{
	const float cX = Cos(x);
	const float sX = Sin(x);
	const float cY = Cos(y);
	const float sY = Sin(y);
	const float cZ = Cos(z);
	const float sZ = Sin(z);

	m[0][0] = cY*cZ;
	m[0][1] = cY*sZ;
	m[0][2] = -sY;

	const float sXsY = sX*sY;
	const float cXsY = cX*sY;

	m[1][0] = sXsY*cZ - cX*sZ;
	m[1][1] = sXsY*sZ + cX*cZ;
	m[1][2] = sX*cY;

	m[2][0] = cXsY*cZ + sX*sZ;
	m[2][1] = cXsY*sZ - sX*cZ;
	m[2][2] = cX*cY;

	return *this;
}

Matrix4& Matrix4::SetRotationX(AngleF f)
{
	m[1][1] = m[2][2] = Cos(f);
	m[1][2] = Sin(f);
	m[2][1] = -m[1][2];

	return *this;
}

Matrix4& Matrix4::SetRotationY(AngleF f)
{
	m[0][0] = m[2][2] = Cos(f);
	m[2][0] = Sin(f);
	m[0][2] = -m[2][0];

	return *this;
}

Matrix4& Matrix4::SetRotationZ(AngleF f)
{
	m[0][0] = m[1][1] = Cos(f);
	m[0][1] = Sin(f);
	m[1][0] = -m[0][1];

	return *this;
}

Matrix4& Matrix4::AddRotationX(AngleF f)
{
	const float s = Sin(f);
	const float c = Cos(f);
	float tmp;

	tmp = m[0][1];
	m[0][1] = m[0][1] * c - m[0][2] * s;
	m[0][2] = tmp*s + m[0][2] * c;

	tmp = m[1][1];
	m[1][1] = m[1][1] * c - m[1][2] * s;
	m[1][2] = tmp*s + m[1][2] * c;

	tmp = m[2][1];
	m[2][1] = m[2][1] * c - m[2][2] * s;
	m[2][2] = tmp*s + m[2][2] * c;

	tmp = m[3][1];
	m[3][1] = m[3][1] * c - m[3][2] * s;
	m[3][2] = tmp*s + m[3][2] * c;

	return *this;
}

Matrix4& Matrix4::AddRotationY(AngleF f)
{
	const float s = Sin(f);
	const float c = Cos(f);
	float tmp;

	tmp = m[0][0];
	m[0][0] = m[0][0] * c + m[0][2] * s;
	m[0][2] = m[0][2] * c - tmp*s;

	tmp = m[1][0];
	m[1][0] = m[1][0] * c + m[1][2] * s;
	m[1][2] = m[1][2] * c - tmp*s;

	tmp = m[2][0];
	m[2][0] = m[2][0] * c + m[2][2] * s;
	m[2][2] = m[2][2] * c - tmp*s;

	tmp = m[3][0];
	m[3][0] = m[3][0] * c + m[3][2] * s;
	m[3][2] = m[3][2] * c - tmp*s;

	return *this;
}

Matrix4& Matrix4::AddRotationZ(AngleF f)
{
	const float s = Sin(f);
	const float c = Cos(f);
	float tmp;

	tmp = m[0][0];
	m[0][0] = m[0][0] * c - m[0][1] * s;
	m[0][1] = tmp*s + m[0][1] * c;

	tmp = m[1][0];
	m[1][0] = m[1][0] * c - m[1][1] * s;
	m[1][1] = tmp*s + m[1][1] * c;

	tmp = m[2][0];
	m[2][0] = m[2][0] * c - m[2][1] * s;
	m[2][1] = tmp*s + m[2][1] * c;

	tmp = m[3][0];
	m[3][0] = m[3][0] * c - m[3][1] * s;
	m[3][1] = tmp*s + m[3][1] * c;

	return *this;
}


Matrix4& Matrix4::AddRotation(AngleF x, AngleF y, AngleF z)
{
	const float cX = Cos(x);
	const float sX = Sin(x);
	const float cY = Cos(y);
	const float sY = Sin(y);
	const float cZ = Cos(z);
	const float sZ = Sin(z);

	const float sXsY = sX*sY;
	const float cXsY = cX*sY;

	const float a1 = sXsY*cZ - cX*sZ;
	const float a2 = sXsY*sZ + cX*cZ;
	const float a3 = cXsY*cZ + sX*sZ;
	const float a4 = cXsY*sZ - sX*cZ;
	const float a5 = cY*cZ;
	const float a6 = cY*sZ;
	const float a7 = sX*cY;
	const float a8 = cX*cY;

	float tp1, tp2;

	tp1 = m[0][0]; tp2 = m[0][1];
	m[0][0] = m[0][0] * a5 + m[0][1] * a1 + m[0][2] * a3;
	m[0][1] = tp1*a6 + m[0][1] * a2 + m[0][2] * a4;
	m[0][2] = tp2*a7 + m[0][2] * a8 - sY*tp1;

	tp1 = m[1][0]; tp2 = m[1][1];
	m[1][0] = m[1][0] * a5 + m[1][1] * a1 + m[1][2] * a3;
	m[1][1] = tp1*a6 + m[1][1] * a2 + m[1][2] * a4;
	m[1][2] = tp2*a7 + m[1][2] * a8 - sY*tp1;

	tp1 = m[2][0]; tp2 = m[2][1];
	m[2][0] = m[2][0] * a5 + m[2][1] * a1 + m[2][2] * a3;
	m[2][1] = tp1*a6 + m[2][1] * a2 + m[2][2] * a4;
	m[2][2] = tp2*a7 + m[2][2] * a8 - sY*tp1;

	tp1 = m[3][0]; tp2 = m[3][1];
	m[3][0] = m[3][0] * a5 + m[3][1] * a1 + m[3][2] * a3;
	m[3][1] = tp1*a6 + m[3][1] * a2 + m[3][2] * a4;
	m[3][2] = tp2*a7 + m[3][2] * a8 - sY*tp1;

	return *this;
}

//******************************************************************
// Fragt die Rotation durch diese Matrix ab
Vector3F Matrix4::GetRotationDeg() const
{
	const Matrix4& tmp = *this;
	const Vector3F scale = GetScale();
	const Vector3F invScale(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);

	float Y = (float)-asin(tmp.m[0][2] * invScale.x);
	const float C = cosf(Y);
	Y *= math::Constants<float>::deg_to_rad();

	float rotX, rotY, X, Z;
	if(!IsZero(C)) {
		const float invC = 1.0f / C;
		rotX = tmp.m[2][2] * invC * invScale.z;
		rotY = tmp.m[1][2] * invC * invScale.y;
		X = (float)math::DegToRad(atan2(rotY, rotX));
		rotX = tmp.m[0][0] * invC * invScale.x;
		rotY = tmp.m[0][1] * invC * invScale.x;
		Z = (float)math::DegToRad(atan2(rotY, rotX));
	} else {
		X = 0.0f;
		rotX = tmp.m[1][1] * invScale.y;
		rotY = -tmp.m[1][0] * invScale.y;
		Z = (float)atan2(rotY, rotX) * math::Constants<float>::deg_to_rad();
	}

	// Die Werte auf 360° zurechtstuzen
	if(X < 0.0f) X += 360.0f;
	if(Y < 0.0f) Y += 360.0f;
	if(Z < 0.0f) Z += 360.0f;

	return Vector3F(X, Y, Z);
}

Matrix4& Matrix4::BuildWorld(const Vector3F& vScale,
	const Vector3F& vRot,
	const Vector3F& vTrans)
{

	const float cX = vRot.x != 0.0f ? cosf(vRot.x) : 1.0f;
	const float sX = vRot.x != 0.0f ? sinf(vRot.x) : 0.0f;

	const float cY = vRot.y != 0.0f ? cosf(vRot.y) : 1.0f;
	const float sY = vRot.y != 0.0f ? sinf(vRot.y) : 0.0f;

	const float cZ = vRot.z != 0.0f ? cosf(vRot.z) : 1.0f;
	const float sZ = vRot.z != 0.0f ? sinf(vRot.z) : 0.0f;

	const float sXsY = sX*sY;
	const float cXsY = cX*sY;

	m[0][0] = vScale.x*cY*cZ;
	m[0][1] = vScale.x*cY*sZ;
	m[0][2] = -sY*vScale.x;
	m[0][3] = 0.0f;

	m[1][0] = vScale.y*(sXsY*cZ - cX*sZ);
	m[1][1] = vScale.y*(sXsY*sZ + cX*cZ);
	m[1][2] = vScale.y*sX*cY;
	m[1][3] = 0.0f;

	m[2][0] = vScale.z*(cXsY*cZ + sX*sZ);
	m[2][1] = vScale.z*(cXsY*sZ - sX*cZ);
	m[2][2] = vScale.z*cX*cY;
	m[2][3] = 0.0f;

	m[3][0] = vTrans.x;
	m[3][1] = vTrans.y;
	m[3][2] = vTrans.z;
	m[3][3] = 1.0f;

	return *this;
}

Matrix4& Matrix4::BuildWorld(const Vector3F& Scale,
	const QuaternionF& Orient,
	const Vector3F& Trans)
{
	/*
	// 57 Aktionen
	m[0][0] = Scale.x*(1.0f - 2.0f*Orient.y*Orient.y - 2.0f*Orient.z*Orient.z);
	m[0][1] = Scale.x*(2.0f*Orient.x*Orient.y + 2.0f*Orient.z*Orient.w);
	m[0][2] = Scale.x*(2.0f*Orient.x*Orient.z - 2.0f*Orient.y*Orient.w);
	m[0][3] = 0.0f;

	m[1][0] = Scale.y*(2.0f*Orient.x*Orient.y - 2.0f*Orient.z*Orient.w);
	m[1][1] = Scale.y*(1.0f - 2.0f*Orient.x*Orient.x - 2.0f*Orient.z*Orient.z);
	m[1][2] = Scale.y*(2.0f*Orient.z*Orient.y + 2.0f*Orient.x*Orient.w);
	m[1][3] = 0.0f;

	m[2][0] = Scale.z*(2.0f*Orient.x*Orient.z + 2.0f*Orient.y*Orient.w);
	m[2][1] = Scale.z*(2.0f*Orient.z*Orient.y - 2.0f*Orient.x*Orient.w);
	m[2][2] = Scale.z*(1.0f - 2.0f*Orient.x*Orient.x - 2.0f*Orient.y*Orient.y);
	m[2][3] = 0.0f;

	m[3][0] = Trans.x;
	m[3][1] = Trans.y;
	m[3][2] = Trans.z;
	m[3][3] = 1.0f;
	*/

	// 36 Aktionen
	const float xy = Orient.x * Orient.y;
	const float yy = Orient.y * Orient.y;
	const float xx = Orient.x * Orient.x;
	const float zw = Orient.z * Orient.w;
	const float zz = Orient.z * Orient.z;
	const float xz = Orient.x * Orient.z;
	const float yw = Orient.y * Orient.w;
	const float zy = Orient.z * Orient.y;
	const float xw = Orient.x * Orient.w;

	const float ScaleX2 = 2.0f * Scale.x;
	const float ScaleY2 = 2.0f * Scale.y;
	const float ScaleZ2 = 2.0f * Scale.z;

	m[0][0] = Scale.x * (1.0f - 2.0f * (yy + zz));
	m[0][1] = ScaleX2 * (xy + zw);
	m[0][2] = ScaleX2 * (xz - yw);
	m[0][3] = 0.0f;

	m[1][0] = ScaleY2 * (xy - zw);
	m[1][1] = Scale.y * (1.0f - 2.0f * (xx + zz));
	m[1][2] = ScaleY2 * (zy + xw);
	m[1][3] = 0.0f;

	m[2][0] = ScaleZ2 * (xz + yw);
	m[2][1] = ScaleZ2 * (zy - xw);
	m[2][2] = Scale.z * (1.0f - 2.0f * (xx + yy));
	m[2][3] = 0.0f;

	m[3][0] = Trans.x;
	m[3][1] = Trans.y;
	m[3][2] = Trans.z;
	m[3][3] = 1.0f;

	return *this;
}

//******************************************************************
//Perspektivische Projektionsmatrix (Hyperbolische Z-Transformation)
Matrix4& Matrix4::BuildProjection_Persp(float fieldOfVison,
	float aspect,
	float fNearPlane,
	float fFarPlane)
{
	//Kotangens von 1/2 FOV
	float Cot = 1.0f / tanf(fieldOfVison*0.5f);
	float Q = fFarPlane / (fFarPlane - fNearPlane);

	m[0][0] = Cot / aspect;    m[0][1] = 0.0f; m[0][2] = 0.0f;             m[0][3] = 0.0f;
	m[1][0] = 0.0f;            m[1][1] = Cot;  m[1][2] = 0.0f;             m[1][3] = 0.0f;
	m[2][0] = 0.0f;            m[2][1] = 0.0f; m[2][2] = Q;             m[2][3] = 1.0f;
	m[3][0] = 0.0f;            m[3][1] = 0.0f; m[3][2] = -fNearPlane*Q; m[3][3] = 0.0f;

	return *this;
}

//******************************************************************
//Orthogonale Projektionsmatrix
Matrix4& Matrix4::BuildProjection_Ortho(float XMax,
	float aspect,
	float fNearPlane,
	float fFarPlane)
{
	float Q = 1.0f / (fFarPlane - fNearPlane);
	m[0][0] = 1.0f / XMax;    m[0][1] = 0.0f;         m[0][2] = 0.0f;             m[0][3] = 0.0f;
	m[1][0] = 0.0f;            m[1][1] = 1.0f / (XMax / aspect);    m[1][2] = 0.0f;             m[1][3] = 0.0f;
	m[2][0] = 0.0f;            m[2][1] = 0.0f;         m[2][2] = Q;             m[2][3] = 0.0f;
	m[3][0] = 0.0f;            m[3][1] = 0.0f;         m[3][2] = -fNearPlane*Q; m[3][3] = 1.0f;

	return *this;
}

//******************************************************************
//Kameramatrix mit Blickrichtung
Matrix4& Matrix4::BuildCamera(const Vector3F& vPos,
	const Vector3F& vDir,
	const Vector3F& upVector) //=Vector3(0.0f,1.0f,0.0f)
{
	Vector3F vZAxis = vDir.Normal();
	Vector3F vXAxis = upVector.Cross(vZAxis).Normal();
	Vector3F vYAxis = vZAxis.Cross(vXAxis).Normal();

	m[0][0] = vXAxis.x;        m[0][1] = vYAxis.x;        m[0][2] = vZAxis.x;        m[0][3] = 0.0f;
	m[1][0] = vXAxis.y;        m[1][1] = vYAxis.y;        m[1][2] = vZAxis.y;        m[1][3] = 0.0f;
	m[2][0] = vXAxis.z;        m[2][1] = vYAxis.z;        m[2][2] = vZAxis.z;        m[2][3] = 0.0f;
	m[3][0] = -vXAxis.Dot(vPos); m[3][1] = -vYAxis.Dot(vPos); m[3][2] = -vZAxis.Dot(vPos); m[3][3] = 1.0f;

	return *this;
}

//******************************************************************
//Kameramatrix mit Blickpunkt
Matrix4& Matrix4::BuildCameraLookAt(const Vector3F& vPos,
	const Vector3F& vLookAt,
	const Vector3F& upVector) //=Vector3(0.0f,1.0f,0.0f)
{
	return BuildCamera(vPos, vLookAt - vPos, upVector);
}

//******************************************************************
//Operatoren
//Addition
Matrix4 Matrix4::operator+(const Matrix4& other) const
{
	return Matrix4(m[0][0] + other.m[0][0], m[0][1] + other.m[0][1], m[0][2] + other.m[0][2], m[0][3] + other.m[0][3],
		m[1][0] + other.m[1][0], m[1][1] + other.m[1][1], m[1][2] + other.m[1][2], m[1][3] + other.m[1][3],
		m[2][0] + other.m[2][0], m[2][1] + other.m[2][1], m[2][2] + other.m[2][2], m[2][3] + other.m[2][3],
		m[3][0] + other.m[3][0], m[3][1] + other.m[3][1], m[3][2] + other.m[3][2], m[3][3] + other.m[3][3]);
}

Matrix4& Matrix4::operator+=(const Matrix4& other)
{
	m[0][0] += other.m[0][0];
	m[0][1] += other.m[0][1];
	m[0][2] += other.m[0][2];
	m[0][3] += other.m[0][3];
	m[1][0] += other.m[1][0];
	m[1][1] += other.m[1][1];
	m[1][2] += other.m[1][2];
	m[1][3] += other.m[1][3];
	m[2][0] += other.m[2][0];
	m[2][1] += other.m[2][1];
	m[2][2] += other.m[2][2];
	m[2][3] += other.m[2][3];
	m[3][0] += other.m[3][0];
	m[3][1] += other.m[3][1];
	m[3][2] += other.m[3][2];
	m[3][3] += other.m[3][3];

	return *this;
}

// Subtraktion
Matrix4 Matrix4::operator-(const Matrix4& other) const
{
	return Matrix4(m[0][0] - other.m[0][0], m[0][1] - other.m[0][1], m[0][2] - other.m[0][2], m[0][3] - other.m[0][3],
		m[1][0] - other.m[1][0], m[1][1] - other.m[1][1], m[1][2] - other.m[1][2], m[1][3] - other.m[1][3],
		m[2][0] - other.m[2][0], m[2][1] - other.m[2][1], m[2][2] - other.m[2][2], m[2][3] - other.m[2][3],
		m[3][0] - other.m[3][0], m[3][1] - other.m[3][1], m[3][2] - other.m[3][2], m[3][3] - other.m[3][3]);
}

Matrix4& Matrix4::operator-=(const Matrix4& other)
{
	m[0][0] -= other.m[0][0];
	m[0][1] -= other.m[0][1];
	m[0][2] -= other.m[0][2];
	m[0][3] -= other.m[0][3];
	m[1][0] -= other.m[1][0];
	m[1][1] -= other.m[1][1];
	m[1][2] -= other.m[1][2];
	m[1][3] -= other.m[1][3];
	m[2][0] -= other.m[2][0];
	m[2][1] -= other.m[2][1];
	m[2][2] -= other.m[2][2];
	m[2][3] -= other.m[2][3];
	m[3][0] -= other.m[3][0];
	m[3][1] -= other.m[3][1];
	m[3][2] -= other.m[3][2];
	m[3][3] -= other.m[3][3];

	return *this;
}

// Multiplikation mit Skalar
Matrix4 Matrix4::operator*(float f) const
{
	return Matrix4(m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
		m[1][0] * f, m[1][1] * f, m[1][2] * f, m[1][3] * f,
		m[2][0] * f, m[2][1] * f, m[2][2] * f, m[2][3] * f,
		m[3][0] * f, m[3][1] * f, m[3][2] * f, m[3][3] * f);
}

Matrix4& Matrix4::operator*=(float f)
{
	m[0][0] *= f; m[1][0] *= f; m[2][0] *= f; m[3][0] *= f;
	m[0][1] *= f; m[1][1] *= f; m[2][1] *= f; m[3][1] *= f;
	m[0][2] *= f; m[1][2] *= f; m[2][2] *= f; m[3][2] *= f;
	m[0][3] *= f; m[1][3] *= f; m[2][3] *= f; m[3][3] *= f;

	return *this;
}

// Multiplikation mit einer anderen Matrix
Matrix4 Matrix4::operator* (const Matrix4& other) const
{
	// Multiplikationen sind sehr langsam
	if(this->IsIdent()) return other;
	if(other.IsIdent()) return *this;

	return Matrix4(m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0] + m[0][3] * other.m[3][0],
		m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1] + m[0][3] * other.m[3][1],
		m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2] + m[0][3] * other.m[3][2],
		m[0][0] * other.m[0][3] + m[0][1] * other.m[1][3] + m[0][2] * other.m[2][3] + m[0][3] * other.m[3][3],

		m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0] + m[1][3] * other.m[3][0],
		m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1] + m[1][3] * other.m[3][1],
		m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2] + m[1][3] * other.m[3][2],
		m[1][0] * other.m[0][3] + m[1][1] * other.m[1][3] + m[1][2] * other.m[2][3] + m[1][3] * other.m[3][3],

		m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0] + m[2][3] * other.m[3][0],
		m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1] + m[2][3] * other.m[3][1],
		m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2] + m[2][3] * other.m[3][2],
		m[2][0] * other.m[0][3] + m[2][1] * other.m[1][3] + m[2][2] * other.m[2][3] + m[2][3] * other.m[3][3],

		m[3][0] * other.m[0][0] + m[3][1] * other.m[1][0] + m[3][2] * other.m[2][0] + m[3][3] * other.m[3][0],
		m[3][0] * other.m[0][1] + m[3][1] * other.m[1][1] + m[3][2] * other.m[2][1] + m[3][3] * other.m[3][1],
		m[3][0] * other.m[0][2] + m[3][1] * other.m[1][2] + m[3][2] * other.m[2][2] + m[3][3] * other.m[3][2],
		m[3][0] * other.m[0][3] + m[3][1] * other.m[1][3] + m[3][2] * other.m[2][3] + m[3][3] * other.m[3][3]);
}

Matrix4& Matrix4::SetByProduct(const Matrix4& a, const Matrix4& b)
{
	m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0],
		m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1],
		m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2],
		m[0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3],

		m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0],
		m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1],
		m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2],
		m[1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3],

		m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0],
		m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1],
		m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2],
		m[2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3],

		m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0],
		m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1],
		m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2],
		m[3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];

	return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& other)
{
	if(this->IsIdent()) return ((*this) = other);
	if(other.IsIdent()) return *this;
	return SetByProduct(*this, other);
}

// Zweisungsoperator
Matrix4& Matrix4::operator=(const Matrix4& other)
{
	// Schleife ist sicherer als memcpy
	if(*this == other) return *this;
	for(int i = 0; i < 16; ++i) {
		(*this)[i] = other[i];
	}
	return *this;
}

// Gleichheitsoperator
bool Matrix4::operator==(const Matrix4& other) const
{
	// Schleife ist sicherer als memcmp()
	// Wegen z.B. -0.0f und +0.0f
	for(int i = 0; i < 16; ++i) {
		if((*this)[i] != other[i])
			return false;
	}
	return true;
}

// Ungleichheitsoperator
bool Matrix4::operator!=(const Matrix4& other) const
{
	return !(*this == other);
}

}
}

