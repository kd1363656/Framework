#pragma once

namespace FWK::Graphics
{
	class Texture;
}

namespace FWK::Struct
{
	struct StaticModelVertex final
	{
		TypeAlias::Math::Vector3 m_position = {};
		TypeAlias::Math::Vector3 m_normal   = {};
		TypeAlias::Math::Vector4 m_tangent  = {};
		TypeAlias::Math::Vector2 m_uv		= {};
	};

	struct ModelMaterialAssetData final
	{
		 ModelMaterialAssetData() = default;
		~ModelMaterialAssetData() = default;

		ModelMaterialAssetData(const ModelMaterialAssetData&)           = default;
		ModelMaterialAssetData(	     ModelMaterialAssetData&&) noexcept = default;

		ModelMaterialAssetData& operator=(const ModelMaterialAssetData&)		    = default;
		ModelMaterialAssetData& operator=(	     ModelMaterialAssetData&&) noexcept = default;
		 
		TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultModelMaterialBaseColorFactor;

		std::wstring m_baseColorTextureFilePath = {};
		std::wstring m_normalTextureFilePath    = {};
		std::wstring m_roughnessTextureFilePath = {};
		std::wstring m_metallicTextureFilePath  = {};

		float m_roughnessFactor = Constant::k_defaultModelMaterialRoughnessFactor;
		float m_metallicFactor   = Constant::k_defaultModelMaterialMetallicFactor;
	};

	struct ModelMaterialRuntimeData final
	{
		 ModelMaterialRuntimeData() = default;
		~ModelMaterialRuntimeData() = default;

		ModelMaterialRuntimeData(const ModelMaterialRuntimeData&)			= delete;
		ModelMaterialRuntimeData(	   ModelMaterialRuntimeData&&) noexcept = default;

		ModelMaterialRuntimeData& operator=(const ModelMaterialRuntimeData&)		   = delete;
		ModelMaterialRuntimeData& operator=(	  ModelMaterialRuntimeData&&) noexcept = default;

		std::shared_ptr<Graphics::Texture> m_baseColorTexture = nullptr;
		std::shared_ptr<Graphics::Texture> m_normalTexture	  = nullptr;
		std::shared_ptr<Graphics::Texture> m_roughnessTexture = nullptr;
		std::shared_ptr<Graphics::Texture> m_metallicTexture  = nullptr;
	};

	struct ModelMaterial final
	{
		 ModelMaterial() = default;
		~ModelMaterial() = default;

		ModelMaterial(const ModelMaterial&)           = delete;
		ModelMaterial(	    ModelMaterial&&) noexcept = default;

		ModelMaterial& operator=(const ModelMaterial&)			 = delete;
		ModelMaterial& operator=(	   ModelMaterial&&) noexcept = default;

		ModelMaterialAssetData   m_modelMaterialAssetData   = {};
		ModelMaterialRuntimeData m_modelMaterialRuntimeData = {};
	};

	// Meshlet1個分の参照範囲情報
	struct StaticModelMeshlet final
	{
		// m_uniqueVertexIndexListの開始位置
		// MeshShaderでは、このOffsetからm_vertexCount個分の元頂点Indexを読む
		std::uint32_t m_vertexOffset = Constant::k_initialMeshletVertexOffset;

		// m_primitiveIndexListの開始位置
		// MeshShader側では、このOffsetからm_triangleCount個分の三角形情報を読む
		std::uint32_t m_triangleOffset = Constant::k_initialMeshletTriangleOffset;

		// このMeshletで使用するユニーク頂点数
		// MeshShaderのSetMeshOutputCounts()で出力頂点数として使用する
		std::uint32_t m_vertexCount = Constant::k_initialMeshletVertexCount;

		// このMeshletに含まれる三角形数
		std::uint32_t m_triangleCount = Constant::k_initialMeshletTriangleCount;
	};

	// Meshlet 1個分のカリング用境界情報
	// Meshlet単位のFrustumCullingやBackface Cone Cullingで使用する
	struct StaticModelMeshletBounds final
	{
		// Meshletを囲むBounding Sphereの中心座標
		// Frustum Cullingで視錐台の中にあるか判定するために使用する
		TypeAlias::Math::Vector3 m_center = {};

		// Meshletを囲むBounding Sphereの半径
		// m_centerと合わせて、Meshletがカメラに映る可能性があるか判定する
		float m_radius = Constant::k_initialMeshletBoundsRadius;

		// Backface Cone Culling用の円錐の頂点座標
		// Meshlet内の三角形群がカメラから見て裏向きか判定するために使用する
		TypeAlias::Math::Vector3 m_coneApex = {};

		// Backface Cone Culling用の判定しきい値
		// m_coneAxisと視線方向の内積判定などで使用する
		float m_coneCutoff = Constant::k_initialMeshletBoundsConeCutoff;

		// Backface Cone Culling用の円錐の向き
		// Meshlet内の三角形群が大体どちらを向いているかを表す
		TypeAlias::Math::Vector3 m_coneAxis = {};

		// GPU側の構造体サイズを16bytes単位に揃えるためのPadding
		// Vector3 + float の形にして、HLSL側のStructuredBufferで扱いやすくする
		float m_padding = Constant::k_initialMeshletBoundsPadding;
	};

	// 1つのModelMeshが持つMeshlet関連データ一式
	// ※ MeshShader描画では、このデータをGPU Buffer化してShader側から参照する
	struct StaticModelMeshletData final
	{
		// Meshlet一覧
		// 1要素がMeshlet1個分の参照範囲情報を表す
		std::vector<StaticModelMeshlet> m_meshletList = {};

		// 各Meshletが使用する元頂点Index一覧
		// Meshlet内のLocalVertexIndexから、ModelVertexList上の頂点Indexへ変換するために使用
		std::vector<std::uint32_t> m_uniqueVertexIndexList = {};

		// Meshlet内の三角形情報
		// 1三角形につき3つのLocalVertexIndexを持つ
		// HLSL側のStructuredBuffer<uint>と要素サイズを合わせるためstd::uint32_tで保持する
		std::vector<std::uint32_t> m_primitiveIndexList = {};

		// Meshletごとのカリング用境界情報
		// m_meshletList[i]に対するBoundsはm_meshletBoundsList[i]に入る
		std::vector<StaticModelMeshletBounds> m_meshletBoundsList = {};
	};

	struct StaticModelMeshRuntimeData final
	{
		 StaticModelMeshRuntimeData() = default;
		~StaticModelMeshRuntimeData() = default;

		StaticModelMeshRuntimeData(const StaticModelMeshRuntimeData&)			= delete;
		StaticModelMeshRuntimeData(	     StaticModelMeshRuntimeData&&) noexcept = default;

		StaticModelMeshRuntimeData& operator=(const StaticModelMeshRuntimeData&)		   = delete;
		StaticModelMeshRuntimeData& operator=(      StaticModelMeshRuntimeData&&) noexcept = default;

		// MeshShaderで頂点情報を参照するためのStructuredBuffer
		Struct::StructuredBufferResource m_vertexBuffer = {};

		// MeshShaderで参照するMeshlet情報用StructuredBuffer
		Struct::StructuredBufferResource m_meshletBuffer = {};

		// Meshlet内LocalVertexIndexからModelVertexIndexへ変換するStructuredBuffer
		Struct::StructuredBufferResource m_uniqueVertexIndexBuffer = {};

		// Meshlet内の三角形を組み立てるためのStructuredBuffer
		Struct::StructuredBufferResource m_primitiveIndexBuffer = {};

		// Meshlet単位のカリング情報を参照するStructuredBuffer
		Struct::StructuredBufferResource m_meshletBoundsBuffer = {};
	};

	struct StaticModelMesh final
	{
		 StaticModelMesh() = default;
		~StaticModelMesh() = default;

		StaticModelMesh(const StaticModelMesh&)			  = delete;
		StaticModelMesh(	  StaticModelMesh&&) noexcept = default;

		StaticModelMesh& operator=(const StaticModelMesh&)			 = delete;
		StaticModelMesh& operator=(	     StaticModelMesh&&) noexcept = default;

		std::vector<StaticModelVertex> m_staticModelVertexList = {};
		std::vector<std::uint32_t>     m_indexList	           = {};

		ModelMaterial m_modelMaterial = {};

		// MeshShaderで描画するためのMeshletData
		// FBX読み込み後に、meshoptimizerで作成し、.asset保存/読み込み対象にする
		StaticModelMeshletData m_modelMeshletData = {};

		// MeshShader描画時にGPU側で参照するBufferResource群
		// .asset保存対象ではなく、実行時にModelDataから作成する
		StaticModelMeshRuntimeData m_modelMeshRuntimeData = {};
	};

	struct StaticModelData final
	{
		 StaticModelData() = default;
		~StaticModelData() = default;

		StaticModelData(const StaticModelData&)			  = delete;
		StaticModelData(	  StaticModelData&&) noexcept = default;

		StaticModelData& operator=(const StaticModelData&)			 = delete;
		StaticModelData& operator=(	     StaticModelData&&) noexcept = default;

		std::vector<StaticModelMesh> m_modelMeshList = {};
	};
}