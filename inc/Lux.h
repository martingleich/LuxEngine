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
//! Containes physics
namespace physics
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

#include "math/aabbox3d.h"
#include "math/angle.h"
#include "math/dimension2d.h"
#include "math/line3d.h"
#include "math/matrix4.h"
#include "math/plane3d.h"
#include "math/quaternion.h"
#include "math/rect.h"
#include "math/Transformation.h"
#include "math/triangle3d.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/Curve.h"

#include "core/ReferenceCounted.h"
#include "core/Logger.h"
#include "core/lxList.h"
#include "core/lxArray.h"
#include "core/lxHashMap.h"
#include "core/Timer.h"
#include "core/lxName.h"
#include "core/lxString.h"
#include "core/lxPool.h"
#include "core/StringConverter.h"
#include "core/lxRandom.h"

#include "io/FileSystem.h"
#include "io/File.h"
#include "io/path.h"
#include "io/INIFile.h"
#include "io/Archive.h"

#include "input/InputSystem.h"
#include "input/InputDevice.h"
#include "input/EventReceiver.h"
#include "input/Keycodes.h"

#include "video/VertexTypes.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/Texture.h"
#include "video/CubeTexture.h"
#include "video/Material.h"
#include "video/VideoDriver.h"
#include "video/MaterialRenderer.h"
#include "video/MaterialLibrary.h"
#include "video/ColorConverter.h"
#include "video/SubMesh.h"
#include "video/SpriteBank.h"
#include "video/RenderStatistics.h"
#include "video/SceneValues.h"
#include "video/SpriteBank.h"

#include "video/images/Image.h"
#include "video/images/ImageSystem.h"
#include "video/images/ImageWriter.h"

#include "video/DrawingCanvas.h"

#include "scene/SceneManager.h"
#include "scene/SceneNode.h"
#include "scene/Transformable.h"
#include "scene/Collider.h"
#include "scene/QueryCallback.h"
#include "scene/Query.h"

#include "scene/components/CameraFPSAnimator.h"

#include "scene/nodes/CameraSceneNode.h"
#include "scene/nodes/MeshSceneNode.h"
#include "scene/nodes/LightSceneNode.h"

#include "scene/mesh/Mesh.h"
#include "scene/mesh/MeshSystem.h"
#include "scene/mesh/GeometryCreatorLib.h"

#include "gui/GUIEnvironment.h"
#include "gui/Font.h"
#include "gui/Window.h"
#include "gui/CursorControl.h"
#include "gui/FontCreator.h"

#include "core/lxID.h"
#include "core/Referable.h"
#include "core/ReferableFactory.h"

#include "resources/Resource.h"
#include "resources/ResourceLoader.h"
#include "resources/ResourceSystem.h"

#include "LuxEngine/LuxDevice.h"

#endif // !INCLUDED_LUX_H