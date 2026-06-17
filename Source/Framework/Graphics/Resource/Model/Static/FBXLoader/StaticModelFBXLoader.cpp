#include "StaticModelFBXLoader.h"

bool FWK::Graphics::StaticModelFBXLoader::LoadStaticModelFile(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord)
{
	auto& l_modelData = a_staticModelRecord.GetREFModelData();

	// ModelDataはコピー代入禁止のため、保持しているModelMeshリストだけを空にする
	l_modelData.m_modelMeshList.clear();

	// FBXファイル全体をufbx_sceneとして読み込む
	auto* l_fbxScene = LoadFBXScene(a_filePath);

	// まず.assetから読み込む
	// .assetが読み込めなければUFBXで読み込む
	if (m_binaryConverter.LoadStaticModelAsset(a_filePath, l_modelData)) { return true; }

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_fbxScene, "FBXシーンの読み込みに失敗したため、StaticModelファイルの読み込みに失敗しました。", false);

	// モデルデータをシーンから抽出
	if (!ExtractModelData(l_fbxScene, l_modelData))
	{		
		// 安全のためシーン情報の破棄
		DestroyFBXScene(l_fbxScene);

		FWK_ASSERT_RETURN_VALUE("FBXシーンからStaticModelDataの抽出に失敗しました。", false);
	}

	// 使用し終わったFBXSceneは破棄する
	DestroyFBXScene(l_fbxScene);

	m_binaryConverter.SaveStaticModelAsset(a_filePath, l_modelData);

	return true;
}

bool FWK::Graphics::StaticModelFBXLoader::ExtractModelData(const ufbx_scene* a_fbxScene, Struct::StaticModelData& a_staticModelData) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxScene, "ufbx_sceneが無効のため、StaticModelDataの抽出に失敗しました。", false);

	// StaticModelではufbx_node::geometry_to_worldを使って
	// NodeTransformを頂点へ焼きこむ。
	// ただし、Camera/Light/BoneなどMeshを持たない要素を除外するため、
	// Nodeを走査してMeshが接続されているNodeだけを処理する
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_fbxScene->nodes.count == Constant::k_emptyModelMeshCount, "FBXシーン内にNodeが存在しないため、StaticModelDataの抽出に失敗しました。", false);

	for (auto l_nodeIndex = 0ULL; l_nodeIndex < a_fbxScene->nodes.count; ++l_nodeIndex)
	{
		// FBXのノード情報を取得
		// ノードにはメッシュ、ライト、カメラ、ボーンなどの情報が入っている
		const auto* l_fbxNode = a_fbxScene->nodes.data[l_nodeIndex];

		if (!l_fbxNode) { continue; }

		// Meshを持っていないNodeはスキップする
		const auto& l_fbxMesh = l_fbxNode->mesh;

		if (!l_fbxMesh) { continue; }

		std::vector<Struct::StaticModelMesh> l_staticModelMeshList = {};

		// ufbx_mesh 1つを、自作フレームワーク側のModelMeshへ変換する
		// 1つのufbx_meshに複数のMaterialがある場合、MaterialごとにModelMeshを分割する
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ExtractModelMeshList(l_fbxNode, l_staticModelMeshList), "ufbx_nodeからStaticModelMeshリストの抽出に失敗しました。", false);

		for (auto& l_staticModelMesh : l_staticModelMeshList)
		{
			if (l_staticModelMesh.m_staticModelVertexList.empty()) { continue; }
			if (l_staticModelMesh.m_indexList.empty())	           { continue; }

			a_staticModelData.m_modelMeshList.emplace_back(std::move(l_staticModelMesh));
		}
	}

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_staticModelData.m_modelMeshList.empty(), "有効なStaticModelMeshが存在しないため、StaticModelDataの抽出に失敗しました。", false);

	return true;
}
bool FWK::Graphics::StaticModelFBXLoader::ExtractModelMeshList(const ufbx_node* a_fbxNode, std::vector<Struct::StaticModelMesh>& a_staticModelMeshList) const
{
	// もし前回モデルを読み込んでいたらそのモデルのメッシュ情報が残ってしまうのでリストをクリア
	a_staticModelMeshList.clear();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxNode, "ufbx_nodeがnullptrのため、StaticModelMeshリストの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_fbxMesh, "ufbx_nodeにMeshが存在しないため、StaticModelMeshリストの抽出に失敗しました。", false);

	// MaterialがないMeshの場合は、MaterialなしのModelMeshとして1つだけ作成する
	if (l_fbxMesh->materials.count == Constant::k_emptyModelMeshCount)
	{
		Struct::StaticModelMesh l_staticModelMesh = {};

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ExtractModelMeshByMaterial(a_fbxNode, k_invalidMaterialIndex, l_staticModelMesh), "MaterialなしStaticModelMeshの抽出に失敗しました。", false);

		if (!l_staticModelMesh.m_staticModelVertexList.empty() &&
			!l_staticModelMesh.m_indexList.empty())
		{
			// Materialが存在しないため、AssetData/RuntimeDataは初期値のままにする
			l_staticModelMesh.m_modelMaterial = {};

			a_staticModelMeshList.emplace_back(std::move(l_staticModelMesh));
		}

		return true;
	}

	for (std::size_t l_materialIndex = 0ULL; l_materialIndex < l_fbxMesh->materials.count; ++l_materialIndex)
	{
		Struct::StaticModelMesh l_staticModelMesh = {};

		// 現在のMaterialIndexを使用しているFaceだけを集めて、1つのModelMeshにする
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ExtractModelMeshByMaterial(a_fbxNode, l_materialIndex, l_staticModelMesh), "Material別StaticModelMeshの抽出に失敗しました。", false);

		// このMaterialを使用しているFaceがなければ描画対象にしない
		if (l_staticModelMesh.m_staticModelVertexList.empty()) { continue; }
		if (l_staticModelMesh.m_indexList.empty())	           { continue; }

		const auto* l_fbxMaterial = l_fbxMesh->materials.data[l_materialIndex];

		// FBXから取得したMaterial情報のうち、.assetへ保存してよいデータだけを設定する
		ExtractModelMaterial(l_fbxMaterial, l_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData);

		// Runtime情報はTextureSystem登録後に決めるため、FBXLoaderでは必ず初期値のままにする
		l_staticModelMesh.m_modelMaterial.m_modelMaterialRuntimeData = {};

		a_staticModelMeshList.emplace_back(std::move(l_staticModelMesh));
	}

	return true;
}
bool FWK::Graphics::StaticModelFBXLoader::ExtractModelMeshByMaterial(const ufbx_node* a_fbxNode, const std::size_t& a_materialIndex, Struct::StaticModelMesh& a_staticModelMesh) const
{
	// モデルメッシュの初期化
	a_staticModelMesh.m_staticModelVertexList.clear();
	a_staticModelMesh.m_indexList.clear	           ();
	
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxNode, "ufbx_nodeがnullptrのため、Material別StaticModelMeshの抽出に失敗しました。", false);

	const auto* l_fbxMesh = a_fbxNode->mesh;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_fbxMesh, "ufbx_nodeにMeshが存在しないため、Material別StaticModelMeshの抽出に失敗しました。", false);

	// Faceはポリゴン面のこと
	// Faceが存在しないMeshは、三角形へ変換する元データがないため失敗扱いにする
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_fbxMesh->faces.count == Constant::k_emptyModelMeshCount, "三角形化できるFaceが存在しないため、Material別StaticModelMeshの抽出に失敗しました。", false);

	// max_face_trianglesは、1つのFaceを三角形化したときに必要になる最大三角形数
	// これが0の場合、三角形化できるFaceがない
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_fbxMesh->max_face_triangles == Constant::k_emptyModelMeshCount, "三角形化できるFaceが存在しないため、Material別StaticModelMeshの抽出に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_materialIndex				 != k_invalidMaterialIndex &&
									  l_fbxMesh->face_material.count != l_fbxMesh->faces.count,
									  "face_material数とFace数が一致しないため、Material別StaticModelMeshの抽出に失敗しました。",
									  false);

	// ufbx_triangulate_face()は、三角形化した頂点インデックスを配列へ書き込む
	// 1三角形は3頂点なので、最大三角形数 * 3の作業用配列を用意する
	const auto& l_triangleIndexListSize = l_fbxMesh->max_face_triangles * Constant::k_triangleVertexCount;

	std::vector<std::uint32_t> l_triangleIndexList = {};

	l_triangleIndexList.resize(l_triangleIndexListSize);

	for (auto l_faceIndex = 0ULL; l_faceIndex < l_fbxMesh->faces.count; ++l_faceIndex)
	{
		// 面情報を取得
		const auto& l_fbxFace = l_fbxMesh->faces.data[l_faceIndex];

		// k_invalidMaterialIndexの場合は、MaterialなしMeshとして全Faceを対象にする
		if (a_materialIndex != k_invalidMaterialIndex)
		{
			// face_material配列を使って、現在処理中のMaterialIndexと一致するFaceだけを対象にする
			// 要は同じマテリアルを使用しない場合処理をスキップする
			const auto& l_faceMaterialIndex = static_cast<std::size_t>(l_fbxMesh->face_material.data[l_faceIndex]);

			if (l_faceMaterialIndex != a_materialIndex) { continue; }
		}

		// FBXのFaceは四角形以上の場合もあるため、描画しやすい三角形リストへ変換する
		// ufbx_triangulate_face(三角形化した頂点インデックスの書き込み先、
		//						 書き込み先配列の要素数、
		//						 三角形化するメッシュ、
		//						 三角形化するFace);
		const auto l_triangleCount = ufbx_triangulate_face(l_triangleIndexList.data(),
														   l_triangleIndexList.size(),
														   l_fbxMesh,
														   l_fbxFace);

		for (auto l_triangleIndex = 0ULL; l_triangleIndex < l_triangleCount; ++l_triangleIndex)
		{
			for (auto l_vertexIndex = 0U; l_vertexIndex < Constant::k_triangleVertexCount; ++l_vertexIndex)
			{
				// l_triangleIndexListには、三角形化後のufbx側頂点インデックスが入っている
				// 三角形番号 * 3 + 頂点番号で、現在処理している三角形の頂点インデックスを取り出す
				// 要するに変換した後の三角形頂点に合わせた座標やuvの変換を行っている
				// 例 : l_triangleIndex = 0ULL,   l_vertexIndex = 2ならl_indexOffset = 2;
				// 例 : l_triangleIndex = 100ULL, l_vertexIndex = 2ならl_indexOffset = 302;
				const auto& l_indexOffset    = (l_triangleIndex * Constant::k_triangleVertexCount) + l_vertexIndex;
				const auto  l_fbxVertexIndex = l_triangleIndexList[l_indexOffset];

				Struct::StaticModelVertex l_staticModelVertex = {};

				// ufbx_load_optsがわで、+Xright/+YForward/+ZUpとcm->m変換を行っている
				// ここではさらにNodeTransformをgeometry_tow_worldで反映する
				l_staticModelVertex.m_position = FetchWorldVertexPosition(a_fbxNode, l_fbxMesh, l_fbxVertexIndex);
				l_staticModelVertex.m_uv	   = FetchVertexUV           (l_fbxMesh, l_fbxVertexIndex);
				l_staticModelVertex.m_normal   = FetchWorldVertexNormal  (a_fbxNode, l_fbxMesh, l_fbxVertexIndex);
				l_staticModelVertex.m_tangent  = FetchWorldVertexTangent (a_fbxNode, l_fbxMesh, l_fbxVertexIndex);

				// 今は重複頂点削除をまだ行わないため、三角形の頂点をそのまま追加する
				// Indexは追加した頂点の順番をそのまま示す
				a_staticModelMesh.m_staticModelVertexList.emplace_back(l_staticModelVertex);
				a_staticModelMesh.m_indexList.emplace_back	          (static_cast<std::uint32_t>(a_staticModelMesh.m_indexList.size()));
			}
		}
	}

	return true;
}
void FWK::Graphics::StaticModelFBXLoader::ExtractModelMaterial(const ufbx_material* a_fbxMaterial, Struct::ModelMaterialAssetData& a_modelMaterialAssetData) const
{
	a_modelMaterialAssetData = {};

	if (!a_fbxMaterial) { return; }

	// PBRのベースカラー係数。
	a_modelMaterialAssetData.m_baseColorFactor = FetchBaseColorFactor(a_fbxMaterial->pbr.base_color);

	// Roughnessは表面の粗さ。
	// 0に近いほど鏡のように鋭く反射日、1に近いほどぼやけた反射になる。
	a_modelMaterialAssetData.m_roughnessFactor = FetchMaterialFactor(a_fbxMaterial->pbr.roughness, Constant::k_defaultModelMaterialRoughnessFactor);

	// Metallicは金属度
	// 0なら非金属、1なら金属としてPBR計算する
	a_modelMaterialAssetData.m_metallicFactor = FetchMaterialFactor(a_fbxMaterial->pbr.metalness, Constant::k_defaultModelMaterialMetallicFactor);

	// ベースカラー
	{
		// BaseColorはPBRMaterialならpbr.base_color,
		auto& l_textureFilePath = a_modelMaterialAssetData.m_baseColorTextureFilePath;

		// 通常のFBXMaterialならfbx.diffuse_colorに入っていることが多い
		l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->pbr.base_color);

		if (l_textureFilePath.empty())
		{
			l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->fbx.diffuse_color);
		}
	}

	// 法線テクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_normalTextureFilePath;

		l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->pbr.normal_map);

		if (l_textureFilePath.empty())
		{
			l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->fbx.normal_map);
		}
	}

	// ラフネステクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_roughnessTextureFilePath;

		l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->pbr.roughness);
	}

	// Metallicテクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_metallicTextureFilePath;

		l_textureFilePath = FetchTextureFilePath(a_fbxMaterial->pbr.metalness);
	}
}

FWK::TypeAlias::Math::Color FWK::Graphics::StaticModelFBXLoader::FetchBaseColorFactor(const ufbx_material_map& a_materialMap) const
{
	if (!a_materialMap.has_value) { return Constant::k_defaultModelMaterialBaseColorFactor; }

	return TypeAlias::Math::Color
	{
		static_cast<float>(a_materialMap.value_vec4.x),
		static_cast<float>(a_materialMap.value_vec4.y),
		static_cast<float>(a_materialMap.value_vec4.z),
		static_cast<float>(a_materialMap.value_vec4.w)
	};
}

float FWK::Graphics::StaticModelFBXLoader::FetchMaterialFactor(const ufbx_material_map& a_materialMap, const float a_defaultValue) const
{
	if (!a_materialMap.has_value) { return a_defaultValue; }

	return static_cast<float>(a_materialMap.value_real);
}

std::wstring FWK::Graphics::StaticModelFBXLoader::FetchTextureFilePath(const ufbx_material_map & a_materialMap) const
{
	const auto* l_fbxTexture = a_materialMap.texture;

	if (!l_fbxTexture) { return {}; }

	std::filesystem::path l_textureFilePath = {};

	if (l_fbxTexture->type != UFBX_TEXTURE_FILE) { return {}; }

	// ufbx_texture_typeがUFBX_TEXTURE_FILEの場合、
	// filename / relative_filenameに画像ファイルパスが入っている
	if (l_fbxTexture->relative_filename.length != k_emptyStringLength)
	{
		l_textureFilePath = ConvertUFBXStringToWString(l_fbxTexture->relative_filename);
	}
	else if (l_fbxTexture->filename.length != k_emptyStringLength)
	{
		l_textureFilePath = ConvertUFBXStringToWString(l_fbxTexture->filename);
	}

	if (l_textureFilePath.empty()) { return {}; }

	// 現在のTextureSystemはPNG読み込み方針なので、
	// FBX内のpng等の参照をエンジンで使うpngパスへ変換する
	l_textureFilePath.replace_extension(Constant::k_lowerPNGExtension);

	return l_textureFilePath.wstring();
}
std::wstring FWK::Graphics::StaticModelFBXLoader::ConvertUFBXStringToWString(const ufbx_string& a_fbxString) const
{
	if (!a_fbxString.data ||
		a_fbxString.length == k_emptyStringLength) 
	{
		return {}; 
	}

	std::string l_string = {};

	l_string.assign(a_fbxString.data, a_fbxString.length);

	return std::filesystem::path(l_string).wstring();
}