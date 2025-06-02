#pragma once

namespace graphics
{
	// 위치+색상만 담는 구조체
	struct VertexPC
	{
		float x, y, z;      // POSITION
		float r, g, b, a;   // COLOR
	};

	// 위치+노멀+UV를 담는 구조체
	struct VertexPNUV
	{
		float x, y, z;      // POSITION
		float nx, ny, nz;   // NORMAL
		float u, v;         // UV
	};
}
