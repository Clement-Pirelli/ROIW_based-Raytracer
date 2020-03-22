#include "AlignedAllocator.h"

//All of this code is from: https://stackoverflow.com/questions/12942548/making-stdvector-allocate-aligned-memory
//_None of it_ is mine

template <typename T, Alignment TAlign, typename U, Alignment UAlign>
inline
bool
operator== (const AlignedAllocator<T, TAlign> &, const AlignedAllocator<U, UAlign> &) noexcept
{
    return TAlign == UAlign;
}

template <typename T, Alignment TAlign, typename U, Alignment UAlign>
inline
bool
operator!= (const AlignedAllocator<T, TAlign> &, const AlignedAllocator<U, UAlign> &) noexcept
{
    return TAlign != UAlign;
}

void *
detail::allocate_aligned_memory(size_t align, size_t size)
{
    assert(align >= sizeof(void *));
    assert(nail::is_power_of_two(align));

    if (size == 0) {
        return nullptr;
    }

    void *ptr = _aligned_malloc(size, align);

    return ptr;
}


void
detail::deallocate_aligned_memory(void *ptr) noexcept
{
    return _aligned_free(ptr);
}