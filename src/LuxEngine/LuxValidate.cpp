#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4.h"

#include "video/Color.h"

namespace lux
{
// Validate sizes and offsets of data layout types.
static_assert(offsetof(math::Vector2F, x) == 0, "Bad offset");
static_assert(offsetof(math::Vector2F, y) == 4, "Bad offset");
static_assert(sizeof(math::Vector2F) == 8, "Bad size");

static_assert(offsetof(math::Vector3F, x) == 0, "Bad offset");
static_assert(offsetof(math::Vector3F, y) == 4, "Bad offset");
static_assert(offsetof(math::Vector3F, z) == 8, "Bad offset");
static_assert(sizeof(math::Vector3F) == 12, "Bad size");

static_assert(offsetof(math::QuaternionF, x) == 0, "Bad offset");
static_assert(offsetof(math::QuaternionF, y) == 4, "Bad offset");
static_assert(offsetof(math::QuaternionF, z) == 8, "Bad offset");
static_assert(offsetof(math::QuaternionF, w) == 12, "Bad offset");
static_assert(sizeof(math::QuaternionF) == 16, "Bad size");

static_assert(offsetof(video::Colorf, r) == 0, "Bad offset");
static_assert(offsetof(video::Colorf, g) == 4, "Bad offset");
static_assert(offsetof(video::Colorf, b) == 8, "Bad offset");
static_assert(offsetof(video::Colorf, a) == 12, "Bad offset");
static_assert(sizeof(video::Colorf) == 16, "Bad size");

static_assert(sizeof(math::Matrix4) == 4 * 4 * 4, "Bad size");
static_assert(sizeof(video::Color) == 4, "Bad size");
static_assert(sizeof(math::Vector2I) == 8, "Bad size");
static_assert(sizeof(math::Vector3I) == 12, "Bad size");
}
