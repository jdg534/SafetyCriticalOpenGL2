#ifndef _RUNTIME_PHASE_ALLOCATOR_H_
#define _RUNTIME_PHASE_ALLOCATOR_H_

#include <mutex>
#include <array>

// this class is LLM generated.


class runtime_phase_allocator
{
public:
	static constexpr size_t pool_size = 1024 * 1024; // 1MB.

	static void initialise();

	static void* allocate(size_t size);
	static void deallocate(void* ptr) noexcept;

private:
	struct block_header
	{
		size_t size;
		bool free;
		block_header* next;
	};

	inline static std::array<std::uint8_t, pool_size> pool{};
	inline static block_header* free_list = nullptr;
	inline static bool initialised = false;
	inline static std::mutex lock;
};

#endif // _RUNTIME_PHASE_ALLOCATOR_H_