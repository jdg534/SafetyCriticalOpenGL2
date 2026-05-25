#include "initialisation_phase_allocator.h"

#include "memory_system.h"

#include <memory>
#include <cstdlib>

void* initialisation_phase_allocator::allocate(size_t size)
{
	if (memory_system::get_phase() == memory_system::phase::runtime)
	{
		memory_system::allocation_violation("Initialisation allocator used at runtime.");
	}
	void* pointer = std::malloc(size);
	if (pointer == nullptr)
	{
		throw std::bad_alloc{};
	}
	return pointer;
}

void initialisation_phase_allocator::deallocate(void* pointer) noexcept
{
	std::free(pointer);
}
