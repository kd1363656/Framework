#pragma once

namespace FWK::Graphics
{
	class PhysicsDebugDynamicVertexBufferUploader final : public DynamicVertexBufferUploaderBase<Struct::VBPhysicsDebug>
	{
	public:

		 PhysicsDebugDynamicVertexBufferUploader() = default;
		~PhysicsDebugDynamicVertexBufferUploader() = default;

		FWK_DEFINE_TYPE_INFO(PhysicsDebugDynamicVertexBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::PhysicsDebugDynamicVertexBufferUploader)