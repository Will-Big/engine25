// editor.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
//
#include <Windows.h>
#include <graphics/GraphicsExports.h>
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

	graphics::IRenderer* renderer = graphics::CreateRenderer();

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
			// ������
			renderer->Render();
		}
	}

	graphics::DestroyRenderer(renderer);

	return 0;
}