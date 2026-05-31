#include "runtime_phase_allocator.h"

#include "memory_system.h"

void runtime_phase_allocator::initialise()
{
	free_list = reinterpret_cast<block_header*>(pool.data());
	free_list->size = pool_size - sizeof(block_header);
	free_list->free = true;
	free_list->next = nullptr;
	initialised = true;
}

void* runtime_phase_allocator::allocate(std::size_t size)
{
	if (!initialised)
	{
		memory_system::allocation_violation("runtime allocator not initialised");
	}

	std::lock_guard<std::mutex> guard(lock);

	block_header* current = free_list;

	while (current)
	{
		if (current->free && current->size >= size)
		{
			const std::size_t remaining = current->size - size;
			if (remaining > sizeof(block_header))
			{
				auto* next = reinterpret_cast<block_header*>(reinterpret_cast<std::uint8_t*>(current + 1) + size);
				next->size = remaining - sizeof(block_header);

				next->free = true;
				next->next = current->next;

				current->next = next;
				current->size = size;
			}
			current->free = false;
			return current + 1;
		}
		current = current->next;
	}
	memory_system::allocation_violation("runtime allocator exhausted");
	return nullptr;
}

void runtime_phase_allocator::deallocate(void* ptr) noexcept
{
	if (!ptr)
	{
		return;
	}

	std::lock_guard<std::mutex> guard(lock);

	auto* header = reinterpret_cast<block_header*>(ptr) - 1;
	header->free = true;

	// Basic coalescing
	block_header* current = free_list;

	while (current && current->next)
	{
		if (current->free &&
			current->next->free)
		{
			current->size += sizeof(block_header) + current->next->size;
			current->next = current->next->next;
		}
		else
		{
			current = current->next;
		}
	}
}
