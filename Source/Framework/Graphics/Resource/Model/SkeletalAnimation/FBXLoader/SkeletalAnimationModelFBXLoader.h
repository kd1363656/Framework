#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelFBXLoader final : public FBXModelLoaderBase
	{
	private:

		using BoneNodeIndexMap         = std::unordered_map<const ufbx_node*,    std::uint32_t>;
		using BoneIndexPaletteIndexMap = std::unordered_map<      std::uint32_t, std::uint32_t>;

	public:

		 SkeletalAnimationModelFBXLoader()          = default;
		~SkeletalAnimationModelFBXLoader() override = default;

		bool LoadSkeletalAnimationModelFile(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const;

	private:

		bool CreateBoneNodeIndexMap(const std::vector<const ufbx_node*>& a_modelBoneNodeList, BoneNodeIndexMap& a_boneNodeIndexMap) const;

		bool CreateModelBone(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_node* a_fbxBoneNode, SkeletalAnimationModelRecord::ModelBone& a_modelBone) const;

		bool ApplyModelVertexBoneInfluence(const BoneNodeIndexMap&                                                   a_boneNodeIndexMap,
                                           const ufbx_mesh*                                                          a_fbxMesh,  
										   const ufbx_skin_deformer*					                             a_fbxSkinDeformer,
			                               const std::uint32_t                                                       a_fbxVertexIndex,
												 BoneIndexPaletteIndexMap&                                           a_boneIndexPaletteIndexMap,
												 std::vector<SkeletalAnimationModelRecord::ModelBonePaletteElement>& a_bonePaletteList,
                                                 SkeletalAnimationModelRecord::ModelVertex&                          a_modelVertex) const;

		void ApplyModelVertexBoneInfluenceSlot(const float                                      a_boneWeight,
                                               const std::uint32_t                              a_slotIndex,
                                               const std::uint32_t                              a_bonePaletteIndex,
                                                     SkeletalAnimationModelRecord::ModelVertex& a_modelVertex) const;

		bool ExtractModelData(const ufbx_scene* a_fbxScene, SkeletalAnimationModelRecord::ModelData& a_modelData) const;

		bool ExtractModelMeshList(const BoneNodeIndexMap& a_boneNodeIndexMap, const ufbx_node* a_fbxNode, std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const;

		bool ExtractModelBoneList(const ufbx_scene* a_fbxScene, BoneNodeIndexMap& a_boneNodeIndexMap, std::vector<SkeletalAnimationModelRecord::ModelBone>& a_modelBoneList) const;

		bool ExtractModelMeshByMaterial(const std::size_t&                             a_materialIndex,
			                            const ufbx_node*                               a_fbxNode,
			                            const BoneNodeIndexMap&                        a_boneNodeIndexMap, 
			                                  SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const;

		bool CollectModelBoneNodes(const ufbx_scene* a_fbxScene, std::vector<const ufbx_node*>& a_modelBoneNodeList) const;

		bool NormalizeModelVertexBoneWeight(SkeletalAnimationModelRecord::ModelVertex& a_modelVertex) const;

		TypeAlias::Math::Vector3 FetchLocalVertexPosition(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector3 FetchLocalVertexNormal  (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector4 FetchLocalVertexTangent (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;

		TypeAlias::Math::Matrix ConvertUFBXMatrixToMatrix(const ufbx_matrix& a_fbxMatrix) const;
		
		static constexpr float k_emptyBoneWeight = 0.0F;

		static constexpr float k_defaultTangentX = 1.0F;
		static constexpr float k_defaultTangentY = 0.0F;
		static constexpr float k_defaultTangentZ = 0.0F;
		static constexpr float k_defaultTangentW = 1.0F;

		// Affine Matrixの各軸行に設定するW成分
		static constexpr float k_affineMatrixAxisW = 0.0F;

		// Affine Matrixの平行移動行に設定するW成分
		static constexpr float k_affineMatrixTranslationW = 1.0F;

		static constexpr std::size_t k_emptyUFBXElementCount   = 0ULL;
		static constexpr std::size_t k_initialUFBXElementIndex = 0ULL;

		static constexpr std::size_t k_invalidMaterialIndex       = std::numeric_limits<std::size_t>::max();
		static constexpr std::size_t k_supportedSkinDeformerCount = 1ULL;
		static constexpr std::size_t k_initialSkinDeformerIndex   = 0ULL;

		static constexpr std::uint32_t k_emptyBoneInfluenceCount = 0U;
		static constexpr std::uint32_t k_initialBoneWeightOffset = 0U;
		static constexpr std::uint32_t k_maxBoneInfluenceCount   = 4U;

		static constexpr std::uint32_t k_firstBoneInfluenceSlot  = 0U;
		static constexpr std::uint32_t k_secondBoneInfluenceSlot = 1U;
		static constexpr std::uint32_t k_thirdBoneInfluenceSlot  = 2U;
		static constexpr std::uint32_t k_fourthBoneInfluenceSlot = 3U;
	};
}