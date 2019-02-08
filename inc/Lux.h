#ifndef INCLUDED_LUX_H
#define INCLUDED_LUX_H

/*! \file */

//! The master namespace, all lux functions are contained in here
namespace lux
{
//! Contains mathematical functions
namespace math
{
}
//! Contains render functions
namespace video
{
}
//! Contains scenegraph functions
namespace scene
{
}
//! Contains gui functions
namespace gui
{
}
//! Contains core logic
namespace core
{
}
//! Containes logic functions
namespace logic
{
}
//! Contains the logging functions
/**
See \ref logging_introduction for more information.
*/
namespace log
{
}
//! Containe io functions
namespace io
{
}

//! Contains input device functions
namespace input
{
}

}

//************************************************************************************************
// Engine-Header einbinden
#include "core/LuxBase.h"

#include "math/AABBox.h"
#include "math/Angle.h"
#include "math/Dimension2.h"
#include "math/Line3.h"
#include "math/Matrix4.h"
#include "math/Plane.h"
#include "math/Quaternion.h"
#include "math/Rect.h"
#include "math/Transformation.h"
#include "math/Triangle3.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "core/ReferenceCounted.h"
#include "core/Logger.h"
#include "core/lxList.h"
#include "core/lxArray.h"
#include "core/lxHashMap.h"
#include "core/lxOrderedSet.h"
#include "core/lxOrderedMap.h"
#include "core/Clock.h"
#include "core/lxName.h"
#include "core/lxString.h"
#include "core/lxPool.h"
#include "core/StringConverter.h"
#include "core/StringBuffer.h"
#include "core/lxRandom.h"

#include "io/FileSystem.h"
#include "io/File.h"
#include "io/Path.h"
#include "io/Archive.h"

#include "input/InputSystem.h"
#include "input/InputDevice.h"
#include "input/Keycodes.h"

#include "video/Color.h"
#include "video/ColorSpaces.h"
#include "video/ColorConverter.h"

#include "video/HardwareBufferManager.h"
#include "video/VertexTypes.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "video/Texture.h"
#include "video/CubeTexture.h"
#include "video/RenderTarget.h"

#include "video/RenderStatistics.h"
#include "video/VideoDriver.h"
#include "video/Renderer.h"

#include "video/Shader.h"
#include "video/Material.h"
#include "video/MaterialLibrary.h"

#include "video/SpriteBank.h"

#include "video/images/Image.h"
#include "video/images/ImageSystem.h"

#include "logic/Logic.h"

#include "scene/Scene.h"

#include "scene/Collider.h"

#include "scene/Query.h"

#include "scene/Node.h"

#include "scene/Curve.h"

#include "core/lxSignal.h"

#include "video/mesh/Geometry.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/MeshSystem.h"

#include "gui/GUISkin.h"
#include "gui/GUIEnvironment.h"
#include "gui/Font.h"
#include "gui/Window.h"
#include "gui/Cursor.h"
#include "gui/FontCreator.h"
#include "gui/GUIRenderer.h"

#include "core/Referable.h"
#include "core/ReferableFactory.h"

#include "core/Resource.h"
#include "core/ResourceSystem.h"

#include "LuxEngine/LuxSystemInfo.h"
#include "LuxEngine/LuxDevice.h"
#include "LuxEngine/LuxAppContext.h"

#endif // !INCLUDED_LUX_H
