// Compiling with this file is much faster, since all the d3d9, and d3dx header must be only included once
// For a correct build the following cpp must not be included in the build.
#include "video\CubeTextureD3D9.cpp"
#include "video\TextureD3D9.cpp"
#include "video\HardwareBufferManagerD3D9.cpp"
#include "video\ShaderD3D9.cpp"
#include "video\VideoDriverD3D9.cpp"