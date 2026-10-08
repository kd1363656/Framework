#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelFBXSkeletonExtractor final
    {
    public:

         SkeletalAnimationModelFBXSkeletonExtractor() = default;
        ~SkeletalAnimationModelFBXSkeletonExtractor() = default;

        bool ExtractModelBoneList(const ufbx_scene* a_fbxScene, std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap, std::vector<Struct::SkeletalAnimationModelBone>& a_modelBoneList) const;

    private:

        bool CreateBoneNodeIndexMap(const std::vector<const ufbx_node*>&                       a_modelBoneNodeList,       std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap)                                                       const;
        bool CreateModelBone       (const std::unordered_map<const ufbx_node*, std::uint32_t>& a_boneNodeIndexMap,  const ufbx_node*                                           a_fbxBoneNode,      Struct::SkeletalAnimationModelBone& a_modelBone) const;

        bool CollectModelBoneNodes(const ufbx_scene* a_fbxScene, std::vector<const ufbx_node*>& a_modelBoneNodeList) const;

        static constexpr std::size_t k_emptySkinClusterCount = 0ULL;
    };
}