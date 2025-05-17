#pragma once
#include <directxtk12/SimpleMath.h>

namespace core
{
    using vec2 = DirectX::SimpleMath::Vector2;
	using vec3 = DirectX::SimpleMath::Vector3;
	using vec4 = DirectX::SimpleMath::Vector4;
	using quat = DirectX::SimpleMath::Quaternion;

	using mat4 = DirectX::SimpleMath::Matrix;

	using int2 = DirectX::XMINT2;
	using int3 = DirectX::XMINT3;
	using int4 = DirectX::XMINT4;

	using uint2 = DirectX::XMUINT2;
	using uint3 = DirectX::XMUINT3;
	using uint4 = DirectX::XMUINT4;

    constexpr float Pi = DirectX::XM_PI;

    constexpr float ToRadians(float degrees) { return DirectX::XMConvertToRadians(degrees); }
    constexpr float ToDegrees(float radians) { return DirectX::XMConvertToDegrees(radians); }
}