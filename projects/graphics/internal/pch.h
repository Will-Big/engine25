#ifndef PCH_H
#define PCH_H

#include "GraphicsExports.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wrl.h>

#include <d3dx12.h>
#include <dxgi1_6.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include "Utils.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#endif //PCH_H
