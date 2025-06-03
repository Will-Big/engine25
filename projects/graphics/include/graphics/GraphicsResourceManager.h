#pragma once

#include "GraphicsTypes.h"

namespace graphics
{
	class Mesh;

	class IGraphicsResourceManager
	{
	public:
		virtual ~IGraphicsResourceManager() = default;

		virtual std::shared_ptr<Mesh> CreateMesh(
            const void* vertexData,
            UINT vertexCount,
            UINT vertexStride,
            const PublicInputElement* inputLayoutData,
            UINT inputLayoutCount,
            const void* indexData,
            UINT indexCount,
            GraphicsFormat indexFormat
        ) = 0;
	};
}
