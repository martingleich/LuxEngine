#include "math/Matrix4.h"

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
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);
const Matrix4 Matrix4::ZERO = math::Matrix4(
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0);

Matrix4::Matrix4()
{
	MakeIdent();
}

Matrix4::Matrix4(const Matrix4& other)
{
	*this = other;
}

bool Matrix4::IsIdent() const
{
	// Start with last row, since most matrices contain translations
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

Matrix4& Matrix4::MakeIdent()
{
	*this = IDENTITY;

	return *this;
}

Matrix4 Matrix4::Get3x3() const
{
	return Matrix4(
		m[0][0], m[0][1], m[0][3], 0,
		m[1][0], m[1][1], m[1][3], 0,
		m[3][0], m[3][1], m[3][3], 0,
		0, 0, 0, 1);
}

float Matrix4::GetTransformDet() const
{
	return
		m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

Matrix4& Matrix4::Transpose()
{
	(*this) = Matrix4(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
	return *this;
}

Matrix4 Matrix4::GetTransposed() const
{
	return Matrix4(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
}

Matrix4& Matrix4::InvertTransform(bool* result)
{
	float invDet = GetTransformDet();
	if(IsZero(invDet))
		return MakeIdent();
	invDet = 1 / invDet;

	*this = this->GetTransformInverted(result);
	return *this;
}

Matrix4 Matrix4::GetTransformInverted(bool* result) const
{
	float invDet = GetTransformDet();
	if(IsZero(invDet)) {
		if(result)
			*result = false;
		return IDENTITY;
	} else {
		if(result)
			*result = true;
	}
	invDet = 1 / invDet;

	Matrix4 out;
	out.m[0][0] = invDet * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
	out.m[0][1] = -invDet * (m[0][1] * m[2][2] - m[0][2] * m[2][1]);
	out.m[0][2] = invDet * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
	out.m[0][3] = 0;
	out.m[1][0] = -invDet * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
	out.m[1][1] = invDet * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
	out.m[1][2] = -invDet * (m[0][0] * m[1][2] - m[0][2] * m[1][0]);
	out.m[1][3] = 0;
	out.m[2][0] = invDet * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	out.m[2][1] = -invDet * (m[0][0] * m[2][1] - m[0][1] * m[2][0]);
	out.m[2][2] = invDet * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
	out.m[2][3] = 0;
	out.m[3][0] = -(m[3][0] * out.m[0][0] + m[3][1] * out.m[1][0] + m[3][2] * out.m[2][0]);
	out.m[3][1] = -(m[3][0] * out.m[0][1] + m[3][1] * out.m[1][1] + m[3][2] * out.m[2][1]);
	out.m[3][2] = -(m[3][0] * out.m[0][2] + m[3][1] * out.m[1][2] + m[3][2] * out.m[2][2]);
	out.m[3][3] = 1;

	return out;
}

Matrix4& Matrix4::SetTranslation(const Vector3F& vTrans)
{
	m[3][0] = vTrans.x; m[3][1] = vTrans.y; m[3][2] = vTrans.z;
	return *this;
}

Matrix4& Matrix4::AddTranslation(const Vector3F& vTrans)
{
	m[0][0] += m[0][3] * vTrans.x;    m[0][1] += m[0][3] * vTrans.y;    m[0][2] += m[0][3] * vTrans.z;
	m[1][0] += m[1][3] * vTrans.x;    m[1][1] += m[1][3] * vTrans.y;    m[1][2] += m[1][3] * vTrans.z;
	m[2][0] += m[2][3] * vTrans.x;    m[2][1] += m[2][3] * vTrans.y;    m[2][2] += m[2][3] * vTrans.z;
	m[3][0] += m[3][3] * vTrans.x;    m[3][1] += m[3][3] * vTrans.y;    m[3][2] += m[3][3] * vTrans.z;

	return *this;
}

Vector3F Matrix4::GetTranslation() const
{
	return Vector3F(m[3][0], m[3][1], m[3][2]);
}

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

Vector3F Matrix4::GetScale() const
{
	// 0 Rotationsfall abfangen
	if(IsZero(m[0][1]) && IsZero(m[0][2]) &&
		IsZero(m[1][0]) && IsZero(m[1][2]) &&
		IsZero(m[2][0]) && IsZero(m[2][1]))
		return Vector3F(m[0][0], m[1][1], m[2][2]);

	// Volle Berechnung nötig
	float XScaling = std::sqrt(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]);
	return Vector3F(
		GetTransformDet() > 0 ? XScaling : -XScaling,
		std::sqrt(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]),
		std::sqrt(m[2][0] * m[0][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]));
}

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

Vector3F Matrix4::GetRotationDeg() const
{
	const Matrix4& tmp = *this;
	const Vector3F scale = GetScale();
	const Vector3F invScale(1 / scale.x, 1 / scale.y, 1 / scale.z);

	float Y = (float)-asin(tmp.m[0][2] * invScale.x);
	const float C = cosf(Y);
	Y *= math::Constants<float>::deg_to_rad();

	float rotX, rotY, X, Z;
	if(!IsZero(C)) {
		const float invC = 1 / C;
		rotX = tmp.m[2][2] * invC * invScale.z;
		rotY = tmp.m[1][2] * invC * invScale.y;
		X = (float)math::DegToRad(atan2(rotY, rotX));
		rotX = tmp.m[0][0] * invC * invScale.x;
		rotY = tmp.m[0][1] * invC * invScale.x;
		Z = (float)math::DegToRad(atan2(rotY, rotX));
	} else {
		X = 0;
		rotX = tmp.m[1][1] * invScale.y;
		rotY = -tmp.m[1][0] * invScale.y;
		Z = (float)atan2(rotY, rotX) * math::Constants<float>::deg_to_rad();
	}

	// Die Werte auf 360° zurechtstuzen
	if(X < 0) X += 360;
	if(Y < 0) Y += 360;
	if(Z < 0) Z += 360;

	return Vector3F(X, Y, Z);
}

Vector3F Matrix4::GetAxisX() const
{
	return Vector3F(m[0][0], m[1][0], m[2][0]);
}

Vector3F Matrix4::GetAxisY() const
{
	return Vector3F(m[0][1], m[1][1], m[2][1]);
}
Vector3F Matrix4::GetAxisZ() const
{
	return Vector3F(m[0][2], m[1][2], m[2][2]);
}

Matrix4& Matrix4::BuildWorld(const Vector3F& vScale,
	const Vector3F& vRot,
	const Vector3F& vTrans)
{
	const float cX = vRot.x != 0 ? std::cos(vRot.x) : 1;
	const float sX = vRot.x != 0 ? std::sin(vRot.x) : 0;

	const float cY = vRot.y != 0 ? std::cos(vRot.y) : 1;
	const float sY = vRot.y != 0 ? std::sin(vRot.y) : 0;

	const float cZ = vRot.z != 0 ? std::cos(vRot.z) : 1;
	const float sZ = vRot.z != 0 ? std::sin(vRot.z) : 0;

	const float sXsY = sX*sY;
	const float cXsY = cX*sY;

	m[0][0] = vScale.x*cY*cZ;
	m[0][1] = vScale.x*cY*sZ;
	m[0][2] = -sY*vScale.x;
	m[0][3] = 0;

	m[1][0] = vScale.y*(sXsY*cZ - cX*sZ);
	m[1][1] = vScale.y*(sXsY*sZ + cX*cZ);
	m[1][2] = vScale.y*sX*cY;
	m[1][3] = 0;

	m[2][0] = vScale.z*(cXsY*cZ + sX*sZ);
	m[2][1] = vScale.z*(cXsY*sZ - sX*cZ);
	m[2][2] = vScale.z*cX*cY;
	m[2][3] = 0;

	m[3][0] = vTrans.x;
	m[3][1] = vTrans.y;
	m[3][2] = vTrans.z;
	m[3][3] = 1;

	return *this;
}

Matrix4& Matrix4::BuildWorld(const Vector3F& scale,
	const QuaternionF& orient,
	const Vector3F& trans)
{
	/*
	// 57 Aktionen
	m[0][0] = scale.x*(1 - 2*orient.y*orient.y - 2*orient.z*orient.z);
	m[0][1] = scale.x*(2*orient.x*orient.y + 2*orient.z*orient.w);
	m[0][2] = scale.x*(2*orient.x*orient.z - 2*orient.y*orient.w);
	m[0][3] = 0;

	m[1][0] = scale.y*(2*orient.x*orient.y - 2*orient.z*orient.w);
	m[1][1] = scale.y*(1 - 2*orient.x*orient.x - 2*orient.z*orient.z);
	m[1][2] = scale.y*(2*orient.z*orient.y + 2*orient.x*orient.w);
	m[1][3] = 0;

	m[2][0] = scale.z*(2*orient.x*orient.z + 2*orient.y*orient.w);
	m[2][1] = scale.z*(2*orient.z*orient.y - 2*orient.x*orient.w);
	m[2][2] = scale.z*(1 - 2*orient.x*orient.x - 2*orient.y*orient.y);
	m[2][3] = 0;

	m[3][0] = trans.x;
	m[3][1] = trans.y;
	m[3][2] = trans.z;
	m[3][3] = 1;
	*/

	// 36 Aktionen
	const float xy = orient.x * orient.y;
	const float yy = orient.y * orient.y;
	const float xx = orient.x * orient.x;
	const float zw = orient.z * orient.w;
	const float zz = orient.z * orient.z;
	const float xz = orient.x * orient.z;
	const float yw = orient.y * orient.w;
	const float zy = orient.z * orient.y;
	const float xw = orient.x * orient.w;

	const float ScaleX2 = 2 * scale.x;
	const float ScaleY2 = 2 * scale.y;
	const float ScaleZ2 = 2 * scale.z;

	m[0][0] = scale.x * (1 - 2 * (yy + zz));
	m[0][1] = ScaleX2 * (xy + zw);
	m[0][2] = ScaleX2 * (xz - yw);
	m[0][3] = 0;

	m[1][0] = ScaleY2 * (xy - zw);
	m[1][1] = scale.y * (1 - 2 * (xx + zz));
	m[1][2] = ScaleY2 * (zy + xw);
	m[1][3] = 0;

	m[2][0] = ScaleZ2 * (xz + yw);
	m[2][1] = ScaleZ2 * (zy - xw);
	m[2][2] = scale.z * (1 - 2 * (xx + yy));
	m[2][3] = 0;

	m[3][0] = trans.x;
	m[3][1] = trans.y;
	m[3][2] = trans.z;
	m[3][3] = 1;

	return *this;
}

Matrix4& Matrix4::BuildProjection_Persp(
	AngleF fieldOfVison,
	float aspect,
	float nearPlane,
	float farPlane)
{
	float Cot = 1 / Tan(fieldOfVison/2);
	float Q = farPlane / (farPlane - nearPlane);

	m[0][0] = Cot / aspect; m[0][1] = 0; m[0][2] = 0;             m[0][3] = 0;
	m[1][0] = 0;            m[1][1] = Cot;  m[1][2] = 0;             m[1][3] = 0;
	m[2][0] = 0;            m[2][1] = 0; m[2][2] = Q;             m[2][3] = 1;
	m[3][0] = 0;            m[3][1] = 0; m[3][2] = -nearPlane*Q; m[3][3] = 0;

	return *this;
}

Matrix4& Matrix4::BuildProjection_Ortho(float xMax,
	float aspect,
	float nearPlane,
	float farPlane)
{
	float Q = 1 / (farPlane - nearPlane);
	m[0][0] = 1 / xMax;    m[0][1] = 0;         m[0][2] = 0;             m[0][3] = 0;
	m[1][0] = 0;           m[1][1] = 1 / (xMax / aspect);    m[1][2] = 0;             m[1][3] = 0;
	m[2][0] = 0;           m[2][1] = 0;         m[2][2] = Q;             m[2][3] = 0;
	m[3][0] = 0;           m[3][1] = 0;         m[3][2] = -nearPlane*Q; m[3][3] = 1;

	return *this;
}

Matrix4& Matrix4::BuildCamera(const Vector3F& vPos,
	const Vector3F& vDir,
	const Vector3F& upVector) //=Vector3(0,1,0)
{
	Vector3F vZAxis = vDir.Normal();
	Vector3F vXAxis = upVector.Cross(vZAxis).Normal();
	Vector3F vYAxis = vZAxis.Cross(vXAxis).Normal();

	m[0][0] = vXAxis.x;        m[0][1] = vYAxis.x;        m[0][2] = vZAxis.x;        m[0][3] = 0;
	m[1][0] = vXAxis.y;        m[1][1] = vYAxis.y;        m[1][2] = vZAxis.y;        m[1][3] = 0;
	m[2][0] = vXAxis.z;        m[2][1] = vYAxis.z;        m[2][2] = vZAxis.z;        m[2][3] = 0;
	m[3][0] = -vXAxis.Dot(vPos); m[3][1] = -vYAxis.Dot(vPos); m[3][2] = -vZAxis.Dot(vPos); m[3][3] = 1;

	return *this;
}

Matrix4& Matrix4::BuildCameraLookAt(const Vector3F& vPos,
	const Vector3F& vLookAt,
	const Vector3F& upVector) //=Vector3(0,1,0)
{
	return BuildCamera(vPos, vLookAt - vPos, upVector);
}

Matrix4 Matrix4::operator+(const Matrix4& other) const
{
	return Matrix4(
		m[0][0] + other.m[0][0], m[0][1] + other.m[0][1], m[0][2] + other.m[0][2], m[0][3] + other.m[0][3],
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

Matrix4 Matrix4::operator-(const Matrix4& other) const
{
	return Matrix4(
		m[0][0] - other.m[0][0], m[0][1] - other.m[0][1], m[0][2] - other.m[0][2], m[0][3] - other.m[0][3],
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

Matrix4 Matrix4::operator*(float f) const
{
	return Matrix4(
		m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
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

Matrix4 Matrix4::operator* (const Matrix4& other) const
{
	Matrix4 out;
	out.SetByProduct(*this, other);
	return out;
}

Matrix4& Matrix4::SetByProduct(const Matrix4& a, const Matrix4& b)
{
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			float s = 0;
			for(int k = 0; k < 4; ++k)
				s += a(r, k) * b(k, c);
			m[r][c] = s;
		}
	}

	return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& other)
{
	Matrix4 out;
	out.SetByProduct(*this, other);
	*this = out;
	return *this;
}

Matrix4& Matrix4::operator=(const Matrix4& other)
{
	if(*this == other) return *this;
	for(int i = 0; i < 16; ++i)
		(*this)[i] = other[i];
	return *this;
}

bool Matrix4::operator==(const Matrix4& other) const
{
	for(int i = 0; i < 16; ++i) {
		if((*this)[i] != other[i])
			return false;
	}
	return true;
}

bool Matrix4::operator!=(const Matrix4& other) const
{
	return !(*this == other);
}

}
}
