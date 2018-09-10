#ifndef TERRACES_MEMORY_HPP
#define TERRACES_MEMORY_HPP

#include <memory>

namespace terraces {
namespace utils {

template<typename T>
class array_deleter {
public:
    void operator()(T* ptr) { ::operator delete[](static_cast<void*>(ptr)); }
};

template<typename T>
std::unique_ptr<T[], array_deleter<T>> make_unique_array(std::size_t size) {
    auto ret = reinterpret_cast<T*>(::operator new[](size * sizeof(T)));
    for (std::size_t i = 0; i < size; ++i) {
        new(ret + i) T;
    }
    return std::unique_ptr<T[], array_deleter<T>>{ret};
}

} // namespace utils
} // namespace terraces

#endif //TERRACES_MEMORY_HPP
