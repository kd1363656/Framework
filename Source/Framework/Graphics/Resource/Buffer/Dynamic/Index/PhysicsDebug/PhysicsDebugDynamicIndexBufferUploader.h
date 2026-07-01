#pragma once

namespace FWK::Graphics
{
	class PhysicsDebugDynamicIndexBufferUploader final : public DynamicIndexBufferUploaderBase
	{
	public:

		 PhysicsDebugDynamicIndexBufferUploader() = default;
		~PhysicsDebugDynamicIndexBufferUploader() = default;

		FWK_DEFINE_TYPE_INFO(PhysicsDebugDynamicIndexBufferUploader, DynamicIndexBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::PhysicsDebugDynamicIndexBufferUploader)