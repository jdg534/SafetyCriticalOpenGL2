#include "memory_system.h"

#include <iostream>

namespace memory_system
{
	static phase current_phase = phase::initialisation;


	void set_phase(phase phase)
	{
		current_phase = phase;
	}

	phase get_phase()
	{
		return current_phase;
	}

	[[noreturn]]
	void allocation_violation(const char* reason)
	{
		std::cerr << "MEMORY ALLOCATION VIOLATION: "
			<< reason
			<< std::endl;

		std::abort();
	}
}
