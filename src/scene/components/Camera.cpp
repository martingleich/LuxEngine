#include "scene/components/Camera.h"
#include "scene/Node.h"
#include "scene/Scene.h"

#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::PerspCamera, "lux.comp.PerspCamera");

namespace lux
{
namespace scene
{

void BaseCamera::Controller::Render(SceneRenderPassHelper* helper)
{
	// In theory only visible cameras should be registers, but better be save
	lxAssert(m_Cam->GetNode() && m_Cam->GetNode()->IsTrulyVisible());

	if(m_Cam->m_Listener)
		m_Cam->m_Listener->PreRender(m_Cam);

	math::Matrix4 view, proj;
	math::ViewFrustum frustum;
	m_Cam->CalculateCamData(view, proj, frustum);

	SceneRenderCamData camData;
	camData.frustum = frustum;
	camData.transform = m_Cam->GetNode()->GetAbsoluteTransform();

	auto video = helper->GetRenderer();

	SceneRenderPassDefaultData passData;
	passData.camData = camData;
	passData.renderTarget = m_Cam->GetRenderTarget();
	passData.onPreRender.Connect([&video, &proj, &view]()
	{
		video->SetTransform(video::ETransform::Projection, proj);
		video->SetTransform(video::ETransform::View, view);
	});

	helper->DefaultRenderScene(passData);

	if(m_Cam->m_Listener)
		m_Cam->m_Listener->PostRender(m_Cam);
}

BaseCamera::BaseCamera() :
	m_Aspect(1.0f),
	m_NearPlane(0.1f),
	m_FarPlane(500.0f),
	m_RenderPriority(0),
	m_Listener(nullptr),
	m_Controller(this)
{
}

void BaseCamera::Register(bool doRegister)
{
	Component::Register(doRegister);

	// Only register if camera is visible, deRegister of unregistered node isn't an error.
	if(auto s = GetScene()) {
		if(doRegister) {
			if(GetNode() && GetNode()->IsTrulyVisible())
				s->RegisterRenderController(&m_Controller, true);
		} else {
			s->RegisterRenderController(&m_Controller, false);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

PerspCamera::PerspCamera() :
	m_FOV(math::AngleF::Degree(60.0f))
{
}

PerspCamera::~PerspCamera()
{
}

void PerspCamera::CalculateCamData(
	math::Matrix4& view,
	math::Matrix4& proj,
	math::ViewFrustum& frustum)
{
	auto n = GetNode();
	math::Vector3F position = n->GetAbsolutePosition();
	math::Vector3F direction = n->FromRelativeDir(math::Vector3F::UNIT_Z);
	math::Vector3F up = n->FromRelativeDir(math::Vector3F::UNIT_Y);

	view.BuildCamera(position, direction, up);
	proj.BuildProjection_Persp(m_FOV, m_Aspect, m_NearPlane, m_FarPlane);
	frustum = math::ViewFrustum::FromPerspCam(view, m_FOV, m_Aspect, m_NearPlane, m_FarPlane);
}

} // namespace scene
} // namespace lux
