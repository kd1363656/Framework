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

        virtual void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) = 0;

        // この描画申請で描画するMeshのMaterialの種類
        // 無効値の場合はMaterialの種類を問わず全てのMeshを描画する(影描画などMaterialを使わない描画)
        virtual TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const { return StaticTypeIDGenerator::k_invalidStaticTypeID; }

    protected:

        // 定数バッファの上書き禁止(定数バッファのインデックスを進めて新しい定数バッファに書き込む方式)
        template <Concept::IsDerivedDynamicBufferUploaderBaseConcept ConstantBufferUploaderType, typename ConstantBufferType>
        void SetupConstantBuffer(const ConstantBufferType&     a_constantBuffer,
                                 const RootSignature&          a_rootSignature,
                                 const DirectCommandList&      a_directCommandList,
                                 const FrameResource&          a_frameResource,
                                 const Enum::RootParameterType a_rootParameterType)
        {
            auto l_constantBufferUploader = a_frameResource.FindPTRDynamicBufferUploader<ConstantBufferUploaderType>().lock();

            FWK_ASSERT_RETURN_IF(!l_constantBufferUploader,                                                "PerObject定数バッファアップローダーが取得できないため、定数バッファのセットに失敗しました。");
            FWK_ASSERT_RETURN_IF(l_constantBufferUploader->GetREFTypeSize() != sizeof(ConstantBufferType), "取得した定数バッファアップローダーの型サイズとGPU転送予定の定数バッファが一致しないため、定数バッファのセットに失敗しました。");

            const auto& l_gpuVirtualAddress = l_constantBufferUploader->Write(a_constantBuffer);

            // SetGraphicsRootConstantBufferView(ルートパラメータ番号、
            //                                   CBVとして参照させるGPU仮想アドレス);
            // SetupConstantBufferView内でRootParameterTagからルートパラメータ番号を取得し、
            // 指定したRootParameterへUploadBuffer上の定数バッファを結びつける
            a_directCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, a_rootParameterType);
        }

        // LOD選択に必要なカメラ情報と画面の高さをRendererから取得し、ModelLODSelectorを準備する
        bool SetupModelLODSelector(const Renderer& a_renderer, const float a_maxScreenSpaceErrorPixel, ModelLODSelector& a_modelLODSelector) const;

        TypeAlias::DescriptorIndex FetchTextureSRVDescriptorIndex(const std::weak_ptr<TextureRecord>& a_textureRecord) const;

        FWK_DEFINE_TYPE_INFO_ROOT(DrawRequestPerObjectBase)
    };
}