#include "scene/particle/affector/AffectorLinearForce.h"
#include "scene/particle/affector/AffectorSwirl.h"
#include "scene/particle/emitter/LookAtEmitter.h"
#include "scene/particle/emitter/NormalEmitter.h"
#include "scene/particle/emitter/StraightEmitter.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS_NAMED(linear, "lux.affector.Linear", lux::scene::LinearForceAffector);
LUX_REGISTER_REFERABLE_CLASS_NAMED(swirl, "lux.affector.Swirl", lux::scene::SwirlAffector);
LUX_REGISTER_REFERABLE_CLASS_NAMED(lookAt, "lux.emitter.LookAt", lux::scene::LookAtEmitter);
LUX_REGISTER_REFERABLE_CLASS_NAMED(normal, "lux.emitter.Normal", lux::scene::NormalEmitter);
LUX_REGISTER_REFERABLE_CLASS_NAMED(straight, "lux.emitter.Straight", lux::scene::StraightEmitter);
