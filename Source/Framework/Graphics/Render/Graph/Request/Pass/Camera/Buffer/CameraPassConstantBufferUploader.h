#pragma once

namespace FWK::Graphics
{
	class CameraPassConstantBufferUploader final : public ConstantBufferUploader<Struct::CBCameraPass>
	{
	public:

		 CameraPassConstantBufferUploader()          = default;
		~CameraPassConstantBufferUploader() override = default;

	private:

		static constexpr UINT64  k_bufferTypeSize = sizeof(Struct::CBCameraPass);

		FWK_DEFINE_TYPE_INFO(CameraPassConstantBufferUploader, ConstantBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ConstantBufferSharedFactory, FWK::Graphics::CameraPassConstantBufferUploader)