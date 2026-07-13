#pragma once

namespace FWK::Struct
{
	struct ModelMaterialAssetData final
	{
		static constexpr TypeAlias::Math::Color k_defaultModelMaterialBaseColorFactor =
		{
			1.0F,
			1.0F,
			1.0F,
			1.0F
		};

		static constexpr float k_defaultModelMaterialRoughnessFactor = 1.0F;
		static constexpr float k_defaultModelMaterialMetallicFactor  = 0.0F;

		 ModelMaterialAssetData() = default;
		~ModelMaterialAssetData() = default;

		TypeAlias::Math::Color m_baseColorFactor = k_defaultModelMaterialBaseColorFactor;

		std::wstring m_baseColorTextureFilePath = {};
		std::wstring m_normalTextureFilePath    = {};
		std::wstring m_roughnessTextureFilePath = {};
		std::wstring m_metallicTextureFilePath  = {};

		float m_roughnessFactor = k_defaultModelMaterialRoughnessFactor;
		float m_metallicFactor  = k_defaultModelMaterialMetallicFactor;
	};

	struct ModelMaterialRuntimeData final
	{
		 ModelMaterialRuntimeData() = default;
		~ModelMaterialRuntimeData() = default;

		ModelMaterialRuntimeData(const ModelMaterialRuntimeData&)			= delete;
		ModelMaterialRuntimeData(	   ModelMaterialRuntimeData&&) noexcept = default;

		ModelMaterialRuntimeData& operator=(const ModelMaterialRuntimeData&)		   = delete;
		ModelMaterialRuntimeData& operator=(	  ModelMaterialRuntimeData&&) noexcept = default;

		// 全てのテクスチャをインスタンス化する前提だが、メモリ共有を行うためstd::shared_ptrで保持する
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
	struct ModelMeshlet final
	{
		static constexpr std::uint32_t k_initialMeshletVertexOffset   = 0U;
		static constexpr std::uint32_t k_initialMeshletTriangleOffset = 0U;
		static constexpr std::uint32_t k_initialMeshletVertexCount    = 0U;
		static constexpr std::uint32_t k_initialMeshletTriangleCount  = 0U;

		// m_uniqueVertexIndexListの開始位置
		// MeshShaderでは、このOffsetからm_vertexCount個分の元頂点Indexを読む
		std::uint32_t m_vertexOffset = k_initialMeshletVertexOffset;

		// このMeshletで使用するユニーク頂点数
		// MeshShaderのSetMeshOutputCounts()で出力頂点数として使用する
		std::uint32_t m_vertexCount = k_initialMeshletVertexCount;

		// m_primitiveIndexListの開始位置
		// MeshShader側では、このOffsetからm_triangleCount個分の三角形情報を読む
		std::uint32_t m_triangleOffset = k_initialMeshletTriangleOffset;

		// このMeshletに含まれる三角形数
		std::uint32_t m_triangleCount = k_initialMeshletTriangleCount;
	};

	// Meshlet 1個分のカリング用境界情報
	// Meshlet単位のFrustumCullingやBackface Cone Cullingで使用する
	struct ModelMeshletBounds final
	{
		static constexpr float k_initialMeshletBoundsRadius     = 0.0F;
		static constexpr float k_initialMeshletBoundsConeCutoff = 0.0F;
		static constexpr float k_initialMeshletBoundsPadding    = 0.0F;

		// Meshletを囲むBounding Sphereの中心座標
		// Frustum Cullingで視錐台の中にあるか判定するために使用する
		TypeAlias::Math::Vector3 m_center = {};

		// Meshletを囲むBounding Sphereの半径
		// m_centerと合わせて、Meshletがカメラに映る可能性があるか判定する
		float m_radius = k_initialMeshletBoundsRadius;

		// Backface Cone Culling用の円錐の頂点座標
		// Meshlet内の三角形群がカメラから見て裏向きか判定するために使用する
		TypeAlias::Math::Vector3 m_coneApex = {};

		// Backface Cone Culling用の判定しきい値
		// m_coneAxisと視線方向の内積判定などで使用する
		float m_coneCutoff = k_initialMeshletBoundsConeCutoff;

		// Backface Cone Culling用の円錐の向き
		// Meshlet内の三角形群が大体どちらを向いているかを表す
		TypeAlias::Math::Vector3 m_coneAxis = {};

		// GPU側の構造体サイズを16bytes単位に揃えるためのPadding
		// Vector3 + float の形にして、HLSL側のStructuredBufferで扱いやすくする
		float m_padding = k_initialMeshletBoundsPadding;
	};

	// 1つのModelMeshが持つMeshlet関連データ一式
	// ※ MeshShader描画では、このデータをGPU Buffer化してShader側から参照する
	struct ModelMeshletData final
	{
		// Meshlet一覧
		// 1要素がMeshlet1個分の参照範囲情報を表す
		std::vector<ModelMeshlet> m_meshletList = {};

		// 各Meshletが使用する元頂点Index一覧
		// Meshlet内のLocalVertexIndexから、ModelVertexList上の頂点Indexへ変換するために使用
		std::vector<std::uint32_t> m_uniqueVertexIndexList = {};

		// Meshlet内の三角形情報
		// meshoptimizerが出力するPrimitiveIndexはuint8_t
		// そのままuint32_tに1個ずつ拡張するとGPUBufferが大きくなるため、
		// 3個のuint8_tのPrimitiveIndexListを1個のuint32_tへPackして保存する
		std::vector<std::uint32_t> m_primitiveIndexList = {};

		// Meshletごとのカリング用境界情報
		// m_meshletList[i]に対するBoundsはm_meshletBoundsList[i]に入る
		std::vector<ModelMeshletBounds> m_meshletBoundsList = {};
	};

	struct ModelMeshRuntimeDataBase
	{
		 ModelMeshRuntimeDataBase() = default;
		~ModelMeshRuntimeDataBase() = default;

		ModelMeshRuntimeDataBase(const ModelMeshRuntimeDataBase&)			= delete;
		ModelMeshRuntimeDataBase(	   ModelMeshRuntimeDataBase&&) noexcept = default;

		ModelMeshRuntimeDataBase& operator=(const ModelMeshRuntimeDataBase&)		   = delete;
		ModelMeshRuntimeDataBase& operator=(      ModelMeshRuntimeDataBase&&) noexcept = default;

		// MeshShaderで頂点情報を参照するためのStructuredBuffer
		Graphics::StaticStructuredBuffer m_vertexBuffer = {};

		// MeshShaderで参照するMeshlet情報用StructuredBuffer
		Graphics::StaticStructuredBuffer m_meshletBuffer = {};

		// Meshlet内LocalVertexIndexからModelVertexIndexへ変換するStructuredBuffer
		Graphics::StaticStructuredBuffer m_uniqueVertexIndexBuffer = {};

		// Meshlet内の三角形を組み立てるためのStructuredBuffer
		Graphics::StaticStructuredBuffer m_primitiveIndexBuffer = {};

		// Meshlet単位のカリング情報を参照するStructuredBuffer
		Graphics::StaticStructuredBuffer m_meshletBoundsBuffer = {};
	};
}