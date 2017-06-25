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
	// Initialize shared font data
	FontImpl::InitFontData();

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(FontLoader));

#ifdef LUX_WINDOWS
	m_FontCreator = LUX_NEW(FontCreatorWin32);
#else
	m_FontCreator = nullptr;
#endif
}

GUIEnvironmentImpl::~GUIEnvironmentImpl()
{
	// Destroy shared font data
	FontImpl::DestroyFontData();
}

StrongRef<FontCreator> GUIEnvironmentImpl::GetFontCreator()
{
	return m_FontCreator;
}

}
}
