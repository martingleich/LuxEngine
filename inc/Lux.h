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
#include "math/curve.h"

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
#include "io/path.h"
#include "io/INIFile.h"
#include "io/Archive.h"

#include "input/InputSystem.h"
#include "input/InputDevice.h"
#include "input/InputReceiver.h"
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
#include "video/MaterialRenderer.h"
#include "video/MaterialLibrary.h"

#include "video/LightData.h"
#include "video/FogData.h"
#include "video/AlphaSettings.h"

#include "video/SpriteBank.h"

#include "video/images/Image.h"
#include "video/images/ImageSystem.h"
#include "video/images/ImageWriter.h"

#include "video/DrawingCanvas.h"

#include "scene/SceneManager.h"
#include "scene/Transformable.h"

#include "scene/collider/Collider.h"

#include "scene/query/QueryCallback.h"
#include "scene/query/Query.h"

#include "scene/Node.h"
#include "scene/components/Camera.h"
#include "scene/components/Light.h"
#include "scene/components/SceneMesh.h"
#include "scene/components/SkyBox.h"

#include "scene/components/CameraControl.h"
#include "scene/components/LinearMoveAnimator.h"
#include "scene/components/RotationAnimator.h"

#include "events/lxSignal.h"
#include "events/lxActions.h"

#include "video/mesh/Geometry.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/MeshSystem.h"

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
