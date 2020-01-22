#include <algo.hpp>

template <typename T>
static std::unordered_map <struct nvmalloc<T>*, nvmalloc<T>> alloc_table;

