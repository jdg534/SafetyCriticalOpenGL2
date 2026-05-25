#include "memory_system.h"

#include "initialisation_phase_allocator.h"
#include "runtime_phase_allocator.h"

#include <cstdint>
#include <memory>

void* operator new(size_t size)
{
	if (memory_system::get_phase() == memory_system::phase::runtime)
	{
		return runtime_allocator::allocate(size);
	}
	return initialisation_allocator::allocate(size);
}

void operator delete(void* pointer) noexcept
{
	if (memory_system::get_phase() == memory_system::phase::runtime)
	{
		return runtime_allocator::deallocate(pointer);
	}
	return initialisation_allocator::deallocate(pointer);
}

void* operator new[](std::size_t size)
{
	if (memory_system::get_phase() == memory_system::phase::runtime)
	{
		return runtime_allocator::allocate(size);
	}
	return initialisation_allocator::allocate(size);
}

void operator delete[](void* pointer) noexcept
{
	if (memory_system::get_phase() == memory_system::phase::runtime)
	{
		return runtime_allocator::deallocate(pointer);
	}
	return initialisation_allocator::deallocate(pointer);
}

