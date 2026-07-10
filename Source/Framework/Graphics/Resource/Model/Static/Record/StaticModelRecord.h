#pragma once

namespace FWK::Graphics
{
	class StaticModelRecord final : public AssetRecordBase
	{
	public:

		struct ModelVertex final
		{
			TypeAlias::Math::Vector3 m_position = {};
			TypeAlias::Math::Vector3 m_normal   = {};
			TypeAlias::Math::Vector4 m_tangent  = {};
			TypeAlias::Math::Vector2 m_uv		= {};
		};

		struct ModelMesh final
		{
			 ModelMesh() = default;
			~ModelMesh() = default;

			ModelMesh(const ModelMesh&)			  = delete;
			ModelMesh(      ModelMesh&&) noexcept = default;

			ModelMesh& operator=(const ModelMesh&)			 = delete;
			ModelMesh& operator=(      ModelMesh&&) noexcept = default;

			std::vector<ModelVertex>   m_modelVertexList = {};
			std::vector<std::uint32_t> m_indexList	     = {};

			Struct::ModelMaterial m_modelMaterial = {};

			// MeshShaderで描画するためのMeshletData
			// FBX読み込み後に、meshoptimizerで作成し、.asset保存/読み込み対象にする
			Struct::ModelMeshletData m_modelMeshletData = {};

			// MeshShader描画時にGPU側で参照するBufferResource群
			// .asset保存対象ではなく、実行時にModelDataから作成する
			Struct::ModelMeshRuntimeData m_modelMeshRuntimeData = {};
		};

		struct ModelData final
		{
			 ModelData() = default;
			~ModelData() = default;

			ModelData(const ModelData&)			  = delete;
			ModelData(      ModelData&&) noexcept = default;

			ModelData& operator=(const ModelData&)			 = delete;
			ModelData& operator=(      ModelData&&) noexcept = default;

			std::vector<ModelMesh> m_modelMeshList = {};
		};

	public:

		 StaticModelRecord()		  = default;
		~StaticModelRecord() override = default;
		
		StaticModelRecord(const StaticModelRecord&)			  = delete;
		StaticModelRecord(	    StaticModelRecord&&) noexcept = default;

		StaticModelRecord& operator=(const StaticModelRecord&)			 = delete;
		StaticModelRecord& operator=(	   StaticModelRecord&&) noexcept = default;

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

		void SetModelData(ModelData&& a_set) { m_modelData = std::move(a_set); }

		const auto& GetREFModelData() const { return m_modelData; }

		auto& GetMutableREFModelData() { return m_modelData; }

	private:

		ModelData m_modelData = {};
	};
}