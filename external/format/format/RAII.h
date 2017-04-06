#pragma once
#include <functional>

namespace format
{
namespace internal
{
	// Helper-Structure to call function on end of scope
	/**
	Usage:
	void aFunction()
	{
		...
		RAII scopeGuard(&destroyResources); // Will call destroyResources at the end of the scope.
		...
	}
	*/
	struct RAII
	{
		std::function<void()> m_Leave;

		template <typename LeaveT>
		RAII(LeaveT leave) :
			m_Leave(leave)
		{
		}

		~RAII()
		{
			m_Leave();
		}
	};
}
}
