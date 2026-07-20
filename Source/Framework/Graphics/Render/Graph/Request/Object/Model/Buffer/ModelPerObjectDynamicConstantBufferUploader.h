#pragma once

namespace FWK::Graphics
{
	class ModelPerObjectDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBModelPerObject>
	{
	public:

		 ModelPerObjectDynamicConstantBufferUploader()          = default;
		~ModelPerObjectDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(ModelPerObjectDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::ModelPerObjectDynamicConstantBufferUploader)