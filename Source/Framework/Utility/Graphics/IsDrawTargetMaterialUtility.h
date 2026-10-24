#pragma once

namespace FWK::Utility
{
    // Materialがシェーダーへ渡せる状態で、かつ描画対象の種類のMaterialかどうかを確認する
    // a_targetMaterialStaticTypeIDが無効値の場合は、Materialの種類を問わない(影描画などMaterialを使わない描画)
    inline bool IsDrawTargetMaterial(const std::shared_ptr<Graphics::MaterialBase>& a_material, const TypeAlias::StaticTypeID a_targetMaterialStaticTypeID)
    {
        if (!a_material) { return false; }

        if (a_material->GetVALGPUDataBufferSRVDescriptorIndex() == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return false; }

        if (a_targetMaterialStaticTypeID == StaticTypeIDGenerator::k_invalidStaticTypeID) { return true; }

        // 型ごとのStaticTypeIDを比べるだけなので、Materialをダウンキャストする必要はない
        return a_material->GetVALStaticTypeID() == a_targetMaterialStaticTypeID;
    }
}