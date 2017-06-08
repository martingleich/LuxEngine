#include "GUIEnvironmentImpl.h"
#include "core/Logger.h"

#include "video/VideoDriver.h"
#include "video/images/ImageSystem.h"
#include "video/MaterialLibrary.h"
#include "video/PipelineSettings.h"
#include "video/MaterialRenderer.h"

#include "gui/FontLoader.h"
#include "gui/FontImpl.h"

#include "gui/FontCreator.h"

#ifdef LUX_WINDOWS
#include "gui/FontCreatorWin32.h"
#endif

namespace lux
{
namespace gui
{

GUIEnvironmentImpl::GUIEnvironmentImpl(
	video::ImageSystem* ImagSys,
	video::VideoDriver* Driver,
	video::MaterialLibrary* matLib) :
	m_MatLibrary(matLib),
	m_ImageSystem(ImagSys),
	m_VideoDriver(Driver)
{
	// Register font material renderer
	video::MaterialRenderer* FontRenderer = m_MatLibrary->CloneMaterialRenderer("font", "transparent");
	video::PipelineSettings ps = FontRenderer->GetPipeline();
	ps.zWriteEnabled = false;
	ps.zBufferFunc = video::EZComparisonFunc::Always;
	ps.backfaceCulling = false;
	ps.lighting = false;
	ps.fogEnabled = false;
	FontRenderer->SetPipeline(ps);

	// Register font loader
	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader)(
		m_ImageSystem,
		m_VideoDriver,
		m_MatLibrary));

#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32)(m_MatLibrary, m_VideoDriver, FontRenderer);
#else
	m_FontCreator = nullptr;
#endif
}

StrongRef<FontCreator> GUIEnvironmentImpl::GetFontCreator()
{
	return m_FontCreator;
}

}
}
