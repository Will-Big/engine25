// editor.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"

#include <Windows.h>
#include <graphics/Window.h>
#include <graphics/Renderer.h>

#pragma comment(lib, "graphics.lib")

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	constexpr int windowWidth = 1366;
	constexpr int windowHeight = 768;

	graphics::Window window(hInstance, L"EditorClass", L"Editor", windowWidth, windowHeight);
	window.Initialize();
	window.Show(SW_SHOW);

	auto renderer = graphics::CreateRenderer();

	renderer->Initialize(window.GetHandle(), windowWidth, windowHeight);

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// 렌더링
			renderer->Render();
		}
	}

	graphics::DestroyRenderer(renderer);

	return 0;
}