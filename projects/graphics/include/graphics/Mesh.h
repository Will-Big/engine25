#pragma once

namespace graphics
{
	namespace internal { class MeshResource; }

	class GRAPHICS_API Mesh
	{
	public:
		explicit Mesh(std::shared_ptr<internal::MeshResource> resource);

	private:
		std::shared_ptr<internal::MeshResource> _resource;
	};
}
