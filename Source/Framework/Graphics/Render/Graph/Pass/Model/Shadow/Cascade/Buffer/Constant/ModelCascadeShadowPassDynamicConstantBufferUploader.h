#pragma once

namespace FWK::Graphics
{
	class ModelCascadeShadowPassDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBModelCascadeShadowPass>
	{
	public:

		 ModelCascadeShadowPassDynamicConstantBufferUploader()          = default;
		~ModelCascadeShadowPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(ModelCascadeShadowPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::ModelCascadeShadowPassDynamicConstantBufferUploader)