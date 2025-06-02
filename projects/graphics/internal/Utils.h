#pragma once

#include <ios>
#include <sstream>
#include <stdexcept>

// HRESULT 실패 시 예외를 던지는 헬퍼 함수
namespace graphics
{
	namespace internal
	{
        inline void ThrowIfFailed(HRESULT hr, const char* expr)
        {
            if (FAILED(hr))
            {
                std::ostringstream oss;
                oss << expr
                    << " failed (HRESULT = 0x" << std::hex << hr << ")";
                throw std::runtime_error(oss.str());
            }
        }
	}
}

// 간편 매크로
#ifdef _DEBUG
	#define THROW_IF_FAILED(hr) graphics::internal::ThrowIfFailed((hr), #hr)
#else
	#define THROW_IF_FAILED(hr) hr
#endif

