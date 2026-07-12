#pragma once

namespace FWK::Converter
{
	class ModelBinaryConverterBase : public BinaryConverterBase
	{
	protected:

		struct ModelMeshBinaryHeader final
		{
			std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
			std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

			std::uint64_t m_baseColorTextureFilePathSize = k_emptyTextureFilePathSize;
			std::uint64_t m_normalTextureFilePathSize    = k_emptyTextureFilePathSize;
			std::uint64_t m_roughnessTextureFilePathSize = k_emptyTextureFilePathSize;
			std::uint64_t m_metallicTextureFilePathSize  = k_emptyTextureFilePathSize;

			std::uint64_t m_meshletCount		   = k_emptyModelMeshletCount;
			std::uint64_t m_uniqueVertexIndexCount = k_emptyModelUniqueVertexIndexCount;
			std::uint64_t m_primitiveIndexCount    = k_emptyModelPrimitiveIndexCount;
			std::uint64_t m_meshletBoundsCount     = k_emptyModelMeshletBoundsCount;
		};

	public:

		 ModelBinaryConverterBase()          = default;
		~ModelBinaryConverterBase() override = default;

	protected:

		bool CanLoadAsset(const std::filesystem::path& a_filePath) const;

		template <typename ModelMeshType>
		ModelMeshBinaryHeader CreateModelMeshBinaryHeader(const ModelMeshType& a_modelMesh) const
		{
			ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

			const auto& l_modelMaterialAssetData = a_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMeshletData       = a_modelMesh.m_modelMeshletData;

			// 頂点・Index数。
			l_modelMeshBinaryHeader.m_vertexCount = a_modelMesh.m_modelVertexList.size();
			l_modelMeshBinaryHeader.m_indexCount  = a_modelMesh.m_indexList.size();

			// Materialが参照しているTexturePathのバイナリ保存サイズ。
			// std::wstringは可変長なので、Headerに保存サイズを持たせておく
			l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
			l_modelMeshBinaryHeader.m_normalTextureFilePathSize    = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
			l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
			l_modelMeshBinaryHeader.m_metallicTextureFilePathSize  = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);

			// Meshlet関連データ数。
			l_modelMeshBinaryHeader.m_meshletCount           = l_modelMeshletData.m_meshletList.size();
			l_modelMeshBinaryHeader.m_uniqueVertexIndexCount = l_modelMeshletData.m_uniqueVertexIndexList.size();
			l_modelMeshBinaryHeader.m_primitiveIndexCount    = l_modelMeshletData.m_primitiveIndexList.size();
			l_modelMeshBinaryHeader.m_meshletBoundsCount     = l_modelMeshletData.m_meshletBoundsList.size();

			return l_modelMeshBinaryHeader;
		}

		template <typename ModelMeshType>
		bool TryReadModelMeshBinaryDataCommon(ModelMeshType& a_modelMesh, std::uint64_t& a_memoryReadOffset) const
		{
			ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

			// ModelMesh単位Headerを読み込む
			// 頂点数、Index数、TexturePathサイズ、Meshlet関連配列数が入っている
			if (!TryReadSingleBinaryData(l_modelMeshBinaryHeader, a_memoryReadOffset)) { return false; }

			// ModelMeshの頂点配列を読み込む
			// StaticModelならStatic用頂点、SkeletalAnimationModelならBoneIndex/BoneWeight付き頂点になる
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_modelVertexList, a_memoryReadOffset)) { return false; }

			// 通常Index配列を読み込む
			// Meshlet生成元、または通常Index描画用のIndex
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_indexCount, a_modelMesh.m_indexList, a_memoryReadOffset)) { return false; }

			auto& l_modelMaterialAssetData = a_modelMesh.m_modelMaterial.m_modelMaterialAssetData;

			// MaterialのPBR係数を読み込む
			// Texture本体ではなく、.assetに直接保存できる固定長データ
			if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_baseColorFactor, a_memoryReadOffset)) { return false; }
			if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_roughnessFactor, a_memoryReadOffset)) { return false; }
			if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_metallicFactor,  a_memoryReadOffset)) { return false; }

			// Materialが参照するTexturePathを読み込む
			// std::wstringは可変長なので、Headerに保存されたサイズを使って読む
			if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize, l_modelMaterialAssetData.m_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }
			if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_normalTextureFilePathSize,    l_modelMaterialAssetData.m_normalTextureFilePath,    a_memoryReadOffset)) { return false; }
			if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize, l_modelMaterialAssetData.m_roughnessTextureFilePath, a_memoryReadOffset)) { return false; }
			if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_metallicTextureFilePathSize,  l_modelMaterialAssetData.m_metallicTextureFilePath,  a_memoryReadOffset)) { return false; }

			// Texture本体は.assetに保存しない
			// RuntimeDataはTextureSystem登録時に別途設定する
			a_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData = {};

			auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

			// Meshlet本体を読み込む
			// 各Meshletの頂点範囲、三角形範囲が入っている
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList, a_memoryReadOffset)) { return false; }

			// Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を読み込む
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList, a_memoryReadOffset)) { return false; }

			// Meshlet内の三角形Index情報を読み込む
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList, a_memoryReadOffset)) { return false; }

			// Meshlet単位のカリング用Boundsを読み込む
			if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList, a_memoryReadOffset)) { return false; }

			return true;
		}

		template <typename ModelMeshType>
		void WriteModelMeshBinaryDataCommon(const ModelMeshType& a_modelMesh, std::uint64_t& a_memoryWriteOffset) const
		{
			const auto& l_modelMeshBinaryHeader = CreateModelMeshBinaryHeader(a_modelMesh);

			// ModelMesh単位Headerを書き込む
			// この後に続く可変長配列やTexturePathのサイズ情報を持つ
			WriteBinaryData(k_singleBinaryElementCount, &l_modelMeshBinaryHeader, a_memoryWriteOffset);

			// ModelMeshの頂点配列を書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_modelVertexList.data(), a_memoryWriteOffset);

			// 通常Index配列を書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_indexCount,  a_modelMesh.m_indexList.data(),       a_memoryWriteOffset);

			const auto& l_modelMaterialAssetData = a_modelMesh.m_modelMaterial.m_modelMaterialAssetData;

			// MaterialのPBR係数を書き込む
			WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_baseColorFactor, a_memoryWriteOffset);
			WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_roughnessFactor, a_memoryWriteOffset);
			WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_metallicFactor,  a_memoryWriteOffset);


			// Materialが参照するTexturePathを書き込む
			// Texture本体は.assetには保存しない
			WriteWStringBinaryData(l_modelMaterialAssetData.m_baseColorTextureFilePath, a_memoryWriteOffset);
			WriteWStringBinaryData(l_modelMaterialAssetData.m_normalTextureFilePath,    a_memoryWriteOffset);
			WriteWStringBinaryData(l_modelMaterialAssetData.m_roughnessTextureFilePath, a_memoryWriteOffset);
			WriteWStringBinaryData(l_modelMaterialAssetData.m_metallicTextureFilePath,  a_memoryWriteOffset);

			const auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

			// Meshlet本体を書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList.data(), a_memoryWriteOffset);

			// Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList.data(), a_memoryWriteOffset);

			// Meshlet内の三角形Index情報を書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList.data(), a_memoryWriteOffset);

			// Meshlet単位のカリング用Boundsを書き込む
			WriteBinaryData(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList.data(), a_memoryWriteOffset);
		}

		template <typename ModelMeshType>
		std::uint64_t CalculateModelMeshBinaryFileSizeCommon(const ModelMeshType& a_modelMesh) const
		{
			using ModelVertexType = typename std::remove_cvref_t<decltype(a_modelMesh.m_modelVertexList)>::value_type;

			const auto& l_modelMaterialAssetData = a_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			const auto& l_modelMeshletData       = a_modelMesh.m_modelMeshletData;

			// ヘッダーサイズの計算(indexSizeなどの入っているヘッダー)
			auto l_modelMeshBinaryFileSize = CalculateBinaryDataSize<ModelMeshBinaryHeader>(k_singleBinaryElementCount);

			// モデル頂点、インデックスリストのサイズ計算
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<ModelVertexType>(a_modelMesh.m_modelVertexList.size());
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>  (a_modelMesh.m_indexList.size());

			// PBR係数のサイズ計算
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<TypeAlias::Math::Color>(k_singleBinaryElementCount);
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);

			// PBR用テクスチャファイルパスのサイズ計算
			l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
			l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
			l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
			l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);

			// メッシュレット、及びそのカリング用Boundの計算
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshlet>      (l_modelMeshletData.m_meshletList.size());
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_modelMeshletData.m_uniqueVertexIndexList.size());
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_modelMeshletData.m_primitiveIndexList.size());
			l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshletBounds>(l_modelMeshletData.m_meshletBoundsList.size());

			return l_modelMeshBinaryFileSize;
		}

		static constexpr std::uint64_t k_emptyTextureFilePathSize         = 0ULL;
		static constexpr std::uint64_t k_emptyModelMeshletCount           = 0ULL;
		static constexpr std::uint64_t k_emptyModelUniqueVertexIndexCount = 0ULL;
		static constexpr std::uint64_t k_emptyModelPrimitiveIndexCount    = 0ULL;
		static constexpr std::uint64_t k_emptyModelMeshletBoundsCount     = 0ULL;
	};
}