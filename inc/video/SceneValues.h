#ifndef INCLUDED_SCENEVALUES_H
#define INCLUDED_SCENEVALUES_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxTypes.h"

#include "video/LightData.h"

#include "math/matrix4.h"

namespace lux
{
namespace scene
{

class SceneValues : public ReferenceCounted
{
public:
	enum EMatrizes
	{
		MAT_WORLD = 0,
		MAT_VIEW,
		MAT_PROJ,

		MAT_WORLD_INV,
		MAT_VIEW_INV,

		MAT_WORLD_VIEW,
		MAT_WORLD_PROJ,
		MAT_VIEW_PROJ,
		MAT_WORLD_VIEW_PROJ,
		MAT_WORLD_VIEW_INV,

		MAT_WORLD_TRANS,
		MAT_VIEW_TRANS,

		MAT_WORLD_INV_TRANS,
		MAT_VIEW_INV_TRANS,

		MAT_WORLD_VIEW_TRANS,
		MAT_WORLD_VIEW_INV_TRANS
	};

public:
	virtual ~SceneValues() {}
	virtual void SetMatrix(EMatrizes type, const math::matrix4& matrix) = 0;
	virtual void SetMatrix(EMatrizes type, const math::matrix4& matrix, const math::matrix4& InvMatrix) = 0;
	virtual const math::matrix4& GetMatrix(EMatrizes type) const = 0;

	virtual u32 AddParam(const string& name, core::Type type) = 0;

	virtual bool SetLight(u32 id, const video::LightData& light) = 0;
	virtual void ClearLights() = 0;
	/*
	to dangerous when there are still shaders referencing this
	virtual void RemoveParam(u32 id) = 0;
	virtual void RemoveAllParams() = 0;
	*/
	virtual u32 GetParamCount() const = 0;

	virtual u32 GetParamID(const string& name) const = 0;

	virtual const string& GetParamName(u32 id) const = 0;

	virtual core::Type GetParamType(u32 id) const = 0;

	virtual const void* GetParamValue(u32 id) const = 0;

	virtual void SetParamValue(u32 id, const void* p) = 0;

	virtual u32 GetSysMatrixCount() const = 0;
};

}
}

#endif