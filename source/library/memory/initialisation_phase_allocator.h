#ifndef _INITIALISATION_PHASE_ALLOCATOR_H_
#define _INITIALISATION_PHASE_ALLOCATOR_H_



class initialisation_allocator
{
public:

	static void* allocate(size_t size);
	static void deallocate(void* pointer) noexcept;
};

#endif // _INITIALISATION_PHASE_ALLOCATOR_H_