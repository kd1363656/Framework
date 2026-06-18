#pragma once

namespace FWK::Struct
{
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
		float m_metallicFactor  = Constant::k_defaultModelMaterialMetallicFactor;
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
}