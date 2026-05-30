#include "initialisation_phase_allocator.h"

#include "memory_system.h"

#include <memory>
#include <cstdlib>

// NOLINTBEGIN(hicpp-no-malloc)
// This is an allocator! malloc() and free() have to be interacted with.

void* initialisation_phase_allocator::allocate(std::size_t size)
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

// NOLINTEND(hicpp-no-malloc)
