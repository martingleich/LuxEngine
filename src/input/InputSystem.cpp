#include "input/InputSystem.h"

#include "input/InputSystemImpl.h"

namespace lux
{
namespace input
{

static StrongRef<InputSystem> g_InputSystem;

void InputSystem::Initialize(InputSystem* input)
{
	if(!input)
		input = LUX_NEW(InputSystemImpl);
	
	g_InputSystem = input;
}

InputSystem* InputSystem::Instance()
{
	return g_InputSystem;
}

void InputSystem::Destroy()
{
	g_InputSystem.Reset();
}

}
}
