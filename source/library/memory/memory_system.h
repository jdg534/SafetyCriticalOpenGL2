#ifndef _MEMORY_SYSTEM_H_
#define _MEMORY_SYSTEM_H_

#include <cstdint>

namespace memory_system
{
	enum class phase : std::uint8_t
	{
		initialisation,
		runtime,
		shutdown
	};

	void set_phase(phase phase);
	phase get_phase();

	[[noreturn]]
	void allocation_violation(const char* reason);
}

#endif // _MEMORY_SYSTEM_H_
