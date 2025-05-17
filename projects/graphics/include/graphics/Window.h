#pragma once
#include <string>

namespace graphics
{
	class GRAPHICS_API Window
	{
	public:
		Window(HINSTANCE hInstance,
			std::wstring className,
			std::wstring title,
			int width, int height);

		bool Initialize();

		void Show(int cmdShow);

		bool PullEvent(MSG& msg);

		HWND GetHandle() const { return _hWnd; }

	private:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static ATOM RegisterWindowClass(HINSTANCE hInstance, const std::wstring& className);

		HINSTANCE    _hInstance;
		std::wstring _className;
		std::wstring _title;
		int          _width;
		int          _height;
		HWND         _hWnd = nullptr;
	};
}
