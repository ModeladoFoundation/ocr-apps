#ifndef _OCR_VECTOR_HPP_
#define _OCR_VECTOR_HPP_

#include "ocr_db_alloc.hpp"

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
            const int capacity;
            int head;
            T *const data;

        public:
            typedef T *iterator;

            Vector(const size_t max_capacity):
                capacity(max_capacity), head(0),
                data(ocrNewArray(T, max_capacity)) { }

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

            void resize(size_t s) {
                throw "RESIZE NOT SUPPORTED FOR OCR VECTORS";
            }

    };

    /** Vector defaulting to capacity of 256 */
    template <typename T>
    struct Vector256: public Vector<T> {
        Vector256(): Vector<T>(256) { }
    };
}

#endif /* _OCR_VECTOR_HPP_ */



