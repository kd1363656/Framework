#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class DrawRequestPerObjectBase
	{
	public:

				 DrawRequestPerObjectBase() = default;
		virtual ~DrawRequestPerObjectBase() = default;

		virtual void BeginFrame() = 0;

		virtual void SetupPerObjectConstantBuffer(const ResourceContext& a_resourceContext, const Renderer& a_renderer) = 0;

	protected:

		// 定数バッファの上書き禁止(定数バッファのインデックスを進めて新しい定数バッファに書き込む方式)
		template <Concept::IsDerivedConstantBufferUploaderBaseConcept ConstantBufferUploaderType, typename ConstantBufferType>
		void SetupConstantBuffer(const RootSignature&	        a_rootSignature,
								const DirectCommandList&      a_directCommandList,
								const FrameResource&	        a_frameResource,
								const ConstantBufferType&     a_constantBuffer,
								const Enum::RootParameterType a_rootParameterType)
		{
			auto l_constantBufferUploader = a_frameResource.FindPTRConstantBufferUploader<ConstantBufferUploaderType>().lock();

			FWK_ASSERT_RETURN_IF_FAILED(!l_constantBufferUploader,																"PerObject定数バッファアップローダーが取得できないため、定数バッファのセットに失敗しました。");
			FWK_ASSERT_RETURN_IF_FAILED(l_constantBufferUploader->GetREFConstantBufferTypeSize() != sizeof(ConstantBufferType), "取得した定数バッファアップローダーの型サイズとGPU転送予定の定数バッファが一致しないため、定数バッファのセットに失敗しました。");

			const auto& l_gpuVirtualAddress = l_constantBufferUploader->WritePerObject(a_constantBuffer);

			// SetGraphicsRootConstantBufferView(ルートパラメータ番号、
			//									 CBVとして参照させるGPU仮想アドレス);
			// SetupConstantBufferView内でRootParameterTagからルートパラメータ番号を取得し、
			// 指定したRootParameterへUploadBuffer上の定数バッファを結びつける
			a_directCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, a_rootParameterType);
		}

		TypeAlias::DescriptorIndex FetchVALTextureSRVDescriptorIndex(const std::shared_ptr<Texture>&	   a_texture,	    const TextureSystem& a_textureSystem, const Enum::DefaultTextureType a_defaultTextureType) const;
		TypeAlias::DescriptorIndex FetchVALTextureSRVDescriptorIndex(const std::shared_ptr<TextureRecord>& a_textureRecord, const TextureSystem& a_textureSystem, const Enum::DefaultTextureType a_defaultTextureType) const;

		static constexpr UINT GetVALDefaultDispatchMeshThreadGroupCountX() { return k_defaultDispatchMeshThreadGroupCountX; }
		static constexpr UINT GetVALDefaultDispatchMeshThreadGroupCountY() { return k_defaultDispatchMeshThreadGroupCountY; }
		static constexpr UINT GetVALDefaultDispatchMeshThreadGroupCountZ() { return k_defaultDispatchMeshThreadGroupCountZ; }

	private:

		static constexpr UINT k_defaultDispatchMeshThreadGroupCountX = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

		FWK_DEFINE_TYPE_INFO_ROOT(DrawRequestPerObjectBase)
	};
}