#ifndef _OCR_VECTOR_HPP_
#define _OCR_VECTOR_HPP_

#include "ocr_db_alloc.hpp"
#include "ocr_relative_ptr.hpp"
#include <cstdlib>
#include <cstdio>

namespace Ocr {
    /**
     * This is our std::vector replacement class.
     * Supports a very limited subset of std::vector functionality,
     * and requires a max capacity to be set when it's constructed.
     * Probably only supports primitive and relative pointer types.
     */
    template <typename T>
    struct Vector {
        private:
            const size_t capacity;
            size_t head;
            const RelPtr<T> data;

        public:
            typedef T *iterator;

            Vector(const size_t initial_size, const size_t max_capacity):
                capacity(max_capacity), head(0),
                data(Ocr::NewArray<T>(max_capacity))
            {
                resize(initial_size);
            }

            Vector(const size_t size): Vector(size, size) { }

            T &operator[](const size_t index) { return data[index]; }

            const T &operator[](const size_t index) const { return data[index]; }

            void push_back(const T& value) { data[head++] = value; }

            size_t size(void) const { return head; }

            iterator begin(void) { return data; }

            iterator end(void) { return data + head; }

            iterator erase(iterator pos) {
                const iterator q = end() - 1;
                for (auto p=pos; p<q; p++) {
                    p[0] = p[1];
                }
                --head;
                return pos;
            }

            void reserve(size_t size) {
                if (size > capacity) {
                    fprintf(stderr, "Resizing past initial capacity not supported for Ocr::Vector. (%zu > %zu)\n", size, capacity);
                    abort();
                }
            }

            void resize(size_t size) {
                reserve(size);
                for (size_t i=head; i<size; i++) {
                    Ocr::TypeInitializer<T>::init(data[i]);
                }
                head = size;
            }

    };

    /** Vector defaulting to capacity of N */
    template <typename T, size_t N>
    struct VectorN: public Vector<T> {
        VectorN(size_t sz = 0): Vector<T>(sz, N) { }
    };

    template <typename T> using Vector16  = VectorN<T, 16>;
    template <typename T> using Vector32  = VectorN<T, 32>;
    template <typename T> using Vector64  = VectorN<T, 64>;
    template <typename T> using Vector128 = VectorN<T, 128>;
    template <typename T> using Vector256 = VectorN<T, 256>;
}

#endif /* _OCR_VECTOR_HPP_ */



