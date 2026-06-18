#pragma once

namespace FWK::Graphics
{
	class CameraPassDrawRequest final : public CachedPassConstantBufferDrawRequestBase<Struct::CBCameraPass>
	{
	public:

		 CameraPassDrawRequest()		  = default;
		~CameraPassDrawRequest() override = default;

		bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

		FWK_DEFINE_TYPE_INFO(CameraPassDrawRequest, DrawRequestPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory , FWK::Graphics::CameraPassDrawRequest)