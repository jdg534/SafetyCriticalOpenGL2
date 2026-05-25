#ifndef _INITIALISATION_PHASE_ALLOCATOR_H_
#define _INITIALISATION_PHASE_ALLOCATOR_H_

#include <cstddef>

class initialisation_phase_allocator
{
public:

	static void* allocate(std::size_t size);
	static void deallocate(void* pointer) noexcept;
};

#endif // _INITIALISATION_PHASE_ALLOCATOR_H_