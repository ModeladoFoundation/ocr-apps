
#ifndef PROXY_ALLOCATOR_H
#define PROXY_ALLOCATOR_H

#include <algorithm>
#include <iostream>
#include <map>

template <typename Allocator >
struct allocation_statistics {
    typedef typename Allocator::value_type value_type;

    struct stats {
        size_t max;
        size_t total;
        size_t current;
        size_t count;

        stats() :
            max(0), total(0), current(0), count(0)
        {
        }

        void add( size_t value ) {
            count++;
            total += value;
            current += value;
            max = std::max(max,current);
        }

        void remove( size_t value ) {
            current -= value;
        }

        void display() {
            std::cout
                << "Total: " << total
                << "; Max: " << max
                << "; Avg: " << total/static_cast<double>(count)
                << std::endl;
        }
    };

    ~allocation_statistics() {
        std::cout << "[" << __PRETTY_FUNCTION__ << "]" << std::endl;
        std::cout << "Element size: " << sizeof(value_type) << std::endl;
        for( auto& allocation: _allocations ) {
            std::cout << allocation.first << ": ";
            allocation.second.display();
        }
        std::cout << "===================================================" << std::endl;
    }

    void add( value_type* ptr, size_t n ) {
        _allocations[n].add(n);
    }

    void remove( value_type* ptr, size_t n ) {
        _allocations[n].remove(n);
    }

    std::map<size_t,stats> _allocations;

    static allocation_statistics& get_instance() {
        static allocation_statistics token;
        return token;
    }
};

template <typename RealAllocator>
struct proxy_allocator {
    typedef RealAllocator                        allocator_type;
    typedef typename RealAllocator::value_type   value_type;
    typedef allocation_statistics<RealAllocator> statistics_type;

    template <typename T>
    struct rebind {
        typedef typename allocator_type::template rebind<T>::other backend_allocator;
        typedef proxy_allocator<backend_allocator> other;
    };

    template < typename Alloc >
    proxy_allocator( const proxy_allocator<Alloc>& other ) :
        _backend( other._backend )
    {
    }

    template< typename... Args >
    explicit proxy_allocator( Args&&... args ) :
        _backend( std::forward<Args>(args)... )
    {
    }

	value_type* allocate( std::size_t n ) noexcept {
        value_type* ptr = _backend.allocate(n);
        statistics_type::get_instance().add(ptr,n);
        return ptr;
	}

	void deallocate( value_type* ptr, std::size_t n ) {
        statistics_type::get_instance().remove(ptr, n);
        _backend.deallocate(ptr,n);
	}

private:
    template <typename Alloc>
    friend struct proxy_allocator;

    allocator_type _backend;
};

#endif // FALLBACK_ALLOCATOR_H

