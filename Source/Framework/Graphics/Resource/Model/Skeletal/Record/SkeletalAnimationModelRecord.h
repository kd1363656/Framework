#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelRecord final : public AssetRecordBase
    {
    public:

        using ModelVertex = Struct::SkeletalAnimationModelVertex;
        using ModelMesh   = Struct::SkeletalAnimationModelMesh;

         SkeletalAnimationModelRecord()          = default;
        ~SkeletalAnimationModelRecord() override = default;

        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

        void SetModelData(Struct::SkeletalAnimationModelData&& a_set) { m_modelData = std::move(a_set); }

        const auto& GetREFModelData() const { return m_modelData; }

        auto& GetMutableREFModelData() { return m_modelData; }

    private:

        Struct::SkeletalAnimationModelData m_modelData = {};
    };
}