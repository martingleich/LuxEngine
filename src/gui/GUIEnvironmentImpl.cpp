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

GUIEnvironmentImpl::GUIEnvironmentImpl()
{
	// Register font material renderer
	video::MaterialRenderer* FontRenderer = video::MaterialLibrary::Instance()->CloneMaterialRenderer("font", "transparent");
	video::PipelineSettings ps = FontRenderer->GetPipeline();
	ps.zWriteEnabled = false;
	ps.zBufferFunc = video::EZComparisonFunc::Always;
	ps.backfaceCulling = false;
	ps.lighting = false;
	ps.fogEnabled = false;
	FontRenderer->SetPipeline(ps);

	// Register font loader
	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader));

#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32)(FontRenderer);
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
