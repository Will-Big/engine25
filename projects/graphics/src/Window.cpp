#include "pch.h"
#include "Window.h"

graphics::Window::Window(HINSTANCE hInstance,
                         std::wstring className,
                         std::wstring title, 
                         int width, int height)
	: _hInstance(hInstance), _className(std::move(className)), _title(std::move(title))
	, _width(width), _height(height)
{
}

bool graphics::Window::Initialize()
{
    if (!RegisterWindowClass(_hInstance, _className))
        return false;

    _hWnd = ::CreateWindowExW(
        0,
        _className.c_str(),
        _title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        _width, _height,
        nullptr, nullptr,
        _hInstance,
        nullptr
    );

    return _hWnd != nullptr;
}

void graphics::Window::Show(int cmdShow)
{
    ::ShowWindow(_hWnd, cmdShow);
}

bool graphics::Window::PullEvent(MSG& msg)
{
	return ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
}

LRESULT CALLBACK graphics::Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(hWnd, &ps);
        ::EndPaint(hWnd, &ps);
    }
    return 0;

    default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

ATOM graphics::Window::RegisterWindowClass(HINSTANCE hInstance, const std::wstring& className)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(NULL_BRUSH);
    wc.lpszClassName = className.c_str();
    return ::RegisterClassExW(&wc);
}