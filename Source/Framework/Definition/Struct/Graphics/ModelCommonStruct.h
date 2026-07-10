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
}