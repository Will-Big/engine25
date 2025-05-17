#pragma once
#include <new>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace core
{
	namespace internal
	{
        // 소멸자 콜백 경량 구조체
        using DestructorFunc = void(*)(void*);

        struct DestructorEntry
		{
            DestructorFunc fn;  // 호출할 소멸자 함수 포인터
            void* obj;  // 소멸시킬 객체 인스턴스의 주소
        };

        // n      : 현재 주소나 크기
        // align  : 원하는 정렬 간격 (보통 2의 거듭제곱)
        inline size_t align_up(size_t n, size_t align)
        {
            return (n + (align - 1)) & ~(align - 1);
        }
	}

    class FrameAllocator
	{
    public:
        explicit FrameAllocator(size_t totalSize);

        ~FrameAllocator();

        // 1) raw 메모리 Allocate
        void* Allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));

        // 2) 생성부터 소멸자 등록까지 한 번에 처리하는 Create<T>
        template<typename T, typename... Args>
        T* Create(Args&&... args);

        void Reset();

    private:
        uint8_t* _start = nullptr;
        uint8_t* _ptr = nullptr;
        uint8_t* _end = nullptr;

        std::vector<internal::DestructorEntry> _destructors;
    };

    inline FrameAllocator::FrameAllocator(size_t totalSize)
    {
	    // OS 할당 (new[] 로 대체 가능)
	    _start = static_cast<uint8_t*>(std::malloc(totalSize));
	    assert(_start && "FrameAllocator: 메모리 할당 실패");
	    _ptr = _start;
	    _end = _start + totalSize;
    }

    inline FrameAllocator::~FrameAllocator()
    {
	    std::free(_start);
    }

    inline void* FrameAllocator::Allocate(std::size_t size, std::size_t alignment)
    {
	    const std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(_ptr);
	    const std::uintptr_t aligned = internal::align_up(curr, alignment);
	    std::uint8_t* nextPtr = reinterpret_cast<std::uint8_t*>(aligned) + size;

	    if (nextPtr > _end) throw std::bad_alloc();
	    void* result = reinterpret_cast<void*>(aligned);
	    _ptr = nextPtr;
	    return result;
    }

    template <typename T, typename ... Args>
    T* FrameAllocator::Create(Args&&... args)
    {
	    // 메모리 확보 + 생성자 호출
	    void* raw = Allocate(sizeof(T), alignof(T));
	    T* obj = new (raw) T(std::forward<Args>(args)...);

	    // non-trivial 타입만 소멸자 등록
	    if constexpr (!std::is_trivially_destructible_v<T>)
	    {
		    _destructors.push_back({
			    [](void* p) { static_cast<T*>(p)->~T(); },
			    obj
		    });
	    }
	    return obj;
    }

    inline void FrameAllocator::Reset()
    {
	    for (const auto& e : _destructors) 
		    e.fn(e.obj);

	    _destructors.clear();
	    _ptr = _start;
    }
}
