#ifndef INCLUDED_LUX_DUAL_QUATERNION_H
#define INCLUDED_LUX_DUAL_QUATERNION_H
#include "math/Quaternion.h"

namespace lux
{
namespace math
{
class DualQuaternion
{
public:
	//! Identity dual quaternion.
	DualQuaternion() :
		q(0,0,0,1),
		qe(0,0,0,0)
	{
	}

	//! Dual quaternion form rotation and translation.
	DualQuaternion(const QuaternionF& quat, const Vector3F& trans)
	{
		q = quat;
		qe.w = -0.5f*( trans.x * quat.x + trans.y * quat.y + trans.z * quat.z);
		qe.x =  0.5f*( trans.x * quat.w + trans.y * quat.z - trans.z * quat.y);
		qe.y =  0.5f*(-trans.x * quat.z + trans.y * quat.w + trans.z * quat.x);
		qe.z =  0.5f*( trans.x * quat.y - trans.y * quat.x + trans.z * quat.w);
	}
	//! Dual quaternion from two quaternions
	DualQuaternion(const QuaternionF& _q, const QuaternionF& _qe)
	{
		q = _q;
		qe = _qe;
	}
	//! Dual quaternion from translation and unit quaternion.
	DualQuaternion(const Vector3F& trans)
	{
		qe.w = 0;
		qe.x = 0.5f*trans.x;
		qe.y = 0.5f*trans.y;
		qe.z = 0.5f*trans.z;
	}

	Vector3F TransformPoint(const Vector3F& p) const
	{
		auto v0 = q.GetImag();
		auto v1 = qe.GetImag();
		auto trans = 2.0f*(v1*q.w - v0*qe.w + v0.Cross(v1));
		return q.Transform(p) + trans;
	}

	Vector3F TransformDir(const Vector3F& d) const
	{
		return q.Transform(d);
	}
	DualQuaternion& Invert()
	{
		q = q.GetInverse();
		qe = -q*qe*q;
		return *this;
	}
	DualQuaternion GetInverse()
	{
		return (DualQuaternion(*this).Invert());
	}
	DualQuaternion& Normalize()
	{
		float s = q.GetLengthSq();
		if(s == 0 || s == 1)
			return *this;
		s = 1/std::sqrt(s);
		return *this *= s;
	}
	DualQuaternion GetNormalized() const
	{
		return DualQuaternion(*this).Normalize();
	}
	DualQuaternion& operator+=(const DualQuaternion& other)
	{
		q += other.q;
		qe += other.qe;
		return *this;
	}
	DualQuaternion operator+(const DualQuaternion& other) const
	{
		return DualQuaternion(*this) += other;
	}
	DualQuaternion& operator-=(const DualQuaternion& other)
	{
		q -= other.q;
		qe -= other.qe;
		return *this;
	}
	DualQuaternion operator-(const DualQuaternion& other) const
	{
		return DualQuaternion(*this) += other;
	}
	DualQuaternion& operator*=(float f)
	{
		q *= f;
		qe *= f;
		return *this;
	}
	DualQuaternion operator*(float f) const
	{
		return DualQuaternion(*this) *= f;
	}

	DualQuaternion& operator*=(const DualQuaternion& other)
	{
		auto newq = q * other.qe;
		auto newqe = q * other.qe + qe * other.q;
		q = newq;
		qe = newqe;
		return *this;
	}
	DualQuaternion operator*(const DualQuaternion& other) const
	{
		return DualQuaternion(*this) *= other;
	}

	DualQuaternion operator-() const
	{
		return DualQuaternion(-q, -qe);
	}

	QuaternionF q; //!< Non dual part
	QuaternionF qe; //!< Dual part
};

inline DualQuaternion operator*(float f, const DualQuaternion& dq)
{
	return DualQuaternion(dq) *= f;
}

void fmtPrint(format::Context& ctx, const DualQuaternion& v, format::Placeholder&)
{
	format::vformat(ctx, "[x={} y={} z={} w={} dual_x={} dual_y={} dual_z={} dual_w={}]",
		v.q.x, 
		v.q.y, 
		v.q.z, 
		v.q.w, 
		v.qe.x, 
		v.qe.y, 
		v.qe.z, 
		v.qe.w);
}

}
}

#endif // #ifndef INCLUDED_LUX_DUAL_QUATERNION_H

