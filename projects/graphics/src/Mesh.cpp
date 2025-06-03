#include "pch.h"
#include "Mesh.h"

graphics::Mesh::Mesh(std::shared_ptr<internal::MeshResource> resource)
	: _resource(std::move(resource))
{
}
