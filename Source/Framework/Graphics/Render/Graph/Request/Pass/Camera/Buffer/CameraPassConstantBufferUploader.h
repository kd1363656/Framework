#pragma once

namespace FWK::Graphics
{
	class CameraPassDynamicConstantBufferUploader final : public DynamicConstantBufferFixedWritePositionUploaderBase<Struct::CBCameraPass>
	{
	public:

		 CameraPassDynamicConstantBufferUploader()          = default;
		~CameraPassDynamicConstantBufferUploader() override = default;

		FWK_DEFINE_TYPE_INFO(CameraPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::CameraPassDynamicConstantBufferUploader)