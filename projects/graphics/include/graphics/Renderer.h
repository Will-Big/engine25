#pragma once

namespace graphics
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		virtual void Initialize(HWND hWnd, UINT width, UINT height) = 0;
		virtual void Render() = 0;
	};

	// extern "C" 팩토리 함수
	extern "C"
	{
		GRAPHICS_API IRenderer* CreateRenderer();
		GRAPHICS_API void      DestroyRenderer(IRenderer* pRenderer);
	}
}