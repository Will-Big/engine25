#pragma once

namespace graphics
{
	// API 비종속적인 데이터 포맷 열거형
	enum class GraphicsFormat
	{
		UNKNOWN,

		// --- 정점 요소(Vertex Element)용 포맷 ---
		// 단일 32비트 부동소수점 (예: 깊이 값 단독, 또는 사용자 정의 데이터)
		R32_FLOAT,
		// 2개의 32비트 부동소수점 (예: 텍스처 좌표 UV)
		R32G32_FLOAT,
		// 3개의 32비트 부동소수점 (예: 위치 XYZ, 법선 XYZ)
		R32G32B32_FLOAT,
		// 4개의 32비트 부동소수점 (예: 색상 RGBA, 위치 XYZW)
		R32G32B32A32_FLOAT,

		// 4개의 8비트 부호 없는 정규화된 정수 (예: 색상 RGBA, 각 채널 0-255 -> 0.0-1.0)
		R8G8B8A8_UNORM,
		// 위와 동일하지만 sRGB 색 공간 (감마 보정 적용)
		R8G8B8A8_UNORM_SRGB,

		// --- 인덱스 버퍼(Index Buffer)용 포맷 ---
		// 16비트 부호 없는 정수
		R16_UINT,
		// 32비트 부호 없는 정수
		R32_UINT,

		// TODO: 앞으로 텍스처나 다른 유형의 데이터에 필요한 포맷들을 여기에 추가할 수 있습니다.
		// 예: D32_FLOAT (깊이 버퍼), BC1_UNORM (압축 텍스처) 등
	};

	struct PublicInputElement
	{
		const char* semanticName{};
		UINT semanticIndex{};
		GraphicsFormat format{};
		UINT inputSlot{};
		UINT alignedByteOffset{};
	};
} // namespace graphics
