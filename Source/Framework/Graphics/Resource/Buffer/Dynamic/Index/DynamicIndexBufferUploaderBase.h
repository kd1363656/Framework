#pragma once

namespace FWK::Graphics
{
	class DynamicIndexBufferUploaderBase : public DynamicBufferUploaderBase
	{
	public:

		DynamicIndexBufferUploaderBase ();
		~DynamicIndexBufferUploaderBase() override;

		bool Create(const Device& a_device) override;

		D3D12_INDEX_BUFFER_VIEW WriteIndexList(const std::vector<std::uint32_t>& a_indexList);

	private:

		static constexpr DXGI_FORMAT k_indexFormat = DXGI_FORMAT_R32_UINT;

		FWK_DEFINE_TYPE_INFO(DynamicIndexBufferUploaderBase, DynamicBufferUploaderBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::DynamicIndexBufferUploaderBase)