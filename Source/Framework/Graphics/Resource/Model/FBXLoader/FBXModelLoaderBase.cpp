#include "FBXModelLoaderBase.h"

ufbx_scene* FWK::Graphics::FBXModelLoaderBase::LoadFBXScene(const std::filesystem::path& a_filePath) const
{
	// FBXファイルとして読み込めるパスか確認する
	// 存在しないファイルや.fbx以外のファイルをufbxへ渡さないための事前チェック
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "FBXファイルが読み込める形式ではありません、FBXシーンの読み込みに失敗しました。", nullptr);

	// ufbx_load_optsは、ufbxでFBXを読み込むときの設定
	const auto& l_loadOptions = CreateFBXLoadOptions();
	
	// ufbx_errorは、読み込み失敗時の詳細情報を受け取るための変数
	// ufbx_load_file()が失敗した場合、この中にエラー理由が入る
	ufbx_error l_error = {};

	const auto& l_filePath = a_filePath.string();

	// 成功するとufbx_scene*が返り、失敗するとnullptrが返る
	// ufbx_load_file(読み込むFBXファイルパス、
	//				  読み込み設定、
	//				  エラー情報の書き込み先);
	auto* l_fbxScene = ufbx_load_file(l_filePath.c_str(), &l_loadOptions, &l_error);

	if (!l_fbxScene)
	{
#if defined(_DEBUG)
		std::array<char, k_errorTextBufferSize> l_errorText = {};

		// ufbx_errorはそのままだと読みにくいため、人が読める文字列へ変換する
		// ufbx_format_error(エラー文字列の書き込み先、
		//					 書き込み先バッファサイズ、
		//					 ufbx_load_fileで取得したエラー情報);
		ufbx_format_error (l_errorText.data(), l_errorText.size(), &l_error);
		OutputDebugStringA(l_errorText.data());

#endif
		FWK_ASSERT_RETURN_VALUE("ufbx_load_fileによるFBXシーンの読み込みに失敗しました。", nullptr);
	}

	return l_fbxScene;
}

void FWK::Graphics::FBXModelLoaderBase::ExtractModelMaterial(const ufbx_material* a_fbxMaterial, Struct::ModelMaterialAssetData& a_modelMaterialAssetData) const
{
	a_modelMaterialAssetData = {};

	if (!a_fbxMaterial) { return; }

	// PBRのベースカラー係数。
	a_modelMaterialAssetData.m_baseColorFactor = FetchBaseColorFactor(a_fbxMaterial->pbr.base_color);

	// Roughnessは表面の粗さ。
	// 0に近いほど鏡のように鋭く反射し、1に近いほどぼやけた反射になる。
	a_modelMaterialAssetData.m_roughnessFactor = FetchMaterialFactor(a_fbxMaterial->pbr.roughness, Struct::ModelMaterialAssetData::k_defaultModelMaterialRoughnessFactor);

	// Metallicは金属度
	// 0なら非金属、1なら金属としてPBR計算する
	a_modelMaterialAssetData.m_metallicFactor = FetchMaterialFactor(a_fbxMaterial->pbr.metalness, Struct::ModelMaterialAssetData::k_defaultModelMaterialMetallicFactor);

	// ベースカラー
	{
		// BaseColorはPBRMaterialならpbr.base_color,
		auto& l_textureFilePath = a_modelMaterialAssetData.m_baseColorTextureFilePath;

		// 通常のFBXMaterialならfbx.diffuse_colorに入っていることが多い
		l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->pbr.base_color);

		if (l_textureFilePath.empty())
		{
			l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->fbx.diffuse_color);
		}
	}

	// 法線テクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_normalTextureFilePath;

		l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->pbr.normal_map);

		if (l_textureFilePath.empty())
		{
			l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->fbx.normal_map);
		}
	}

	// ラフネステクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_roughnessTextureFilePath;

		l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->pbr.roughness);
	}

	// Metallicテクスチャ
	{
		auto& l_textureFilePath = a_modelMaterialAssetData.m_metallicTextureFilePath;

		l_textureFilePath = FetchMaterialTextureFilePath(a_fbxMaterial->pbr.metalness);
	}
}

void FWK::Graphics::FBXModelLoaderBase::DestroyFBXScene(ufbx_scene* a_fbxScene) const
{
	if (!a_fbxScene) { return; }

	// ufbx_load_file()で作成されたufbx_sceneは、使い終わったらufbx_free_scene()で解放する
	// 今回はModelDataへ必要な情報をコピーした後、StaticModelFBXLoader側でこの関数を呼ぶ
	ufbx_free_scene(a_fbxScene);
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::FetchWorldVertexPosition(const ufbx_node* a_fbxNode, const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode, "ufbx_nodeがnullptrのため、ワールド頂点座標の取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ワールド頂点座標の取得に失敗しました。", {});

	const auto& l_localPosition = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_position, a_vertexIndex);

	// geometry_to_worldは、Geometry空間からWorld空間へ変換する行列
	// target_axes/target_unit_metersの結果も、ufbx側の変換済み空間として扱う。
	const auto& l_worldPosition = ufbx_transform_position(&a_fbxNode->geometry_to_world, l_localPosition);

	return Utility::ConvertUFBXVector3ToVector3(l_worldPosition);
}
FWK::TypeAlias::Math::Vector2 FWK::Graphics::FBXModelLoaderBase::FetchVertexUV(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、UVの取得に失敗しました。", {});

	// FBXによってはUVが入っていない場合がある
	// その場合は今は空のVector2を返し、読み込み自体は続行する
	if (!a_fbxMesh->vertex_uv.exists) { return {}; }

	// ufbx_get_vertex_vec2()で、指定した頂点インデックスのUVを取得する
	// ufbx_mesh::vertex_uvには、FBX内のUV座標データが入っている
	const auto& l_uv = ufbx_get_vertex_vec2(&a_fbxMesh->vertex_uv, a_vertexIndex);

	auto l_convertedUV = Utility::ConvertUFBXVector2ToVector2(l_uv);

	// DirectXのUV座標に合わせるため、V座標を反転する
	// BlenderなどのDCCツールとDirectXでは、テクスチャの上下方向の扱いが異なる場合がある
	l_convertedUV.y = k_uvCoordinateMax - l_convertedUV.y;

	return l_convertedUV;
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::FetchWorldVertexNormal(const ufbx_node* a_fbxNode, const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode, "ufbx_nodeがnullptrのため、ワールド頂点法線の取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ワールド頂点法線の取得に失敗しました。", {});

	if (!a_fbxMesh->vertex_normal.exists) { return {}; }

	const auto& l_localNormal = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_normal, a_vertexIndex);

	// 法線は通常の位置変換行列をそのまま使わない。
	// ufbx_matrix_for_normals()で法線用行列を作る
	const auto l_normalMatrix = ufbx_matrix_for_normals(&a_fbxNode->geometry_to_world);

	auto l_worldNormal = ufbx_transform_direction(&l_normalMatrix, l_localNormal);

	l_worldNormal = ufbx_vec3_normalize(l_worldNormal);

	return Utility::ConvertUFBXVector3ToVector3(l_worldNormal);
}
FWK::TypeAlias::Math::Vector4 FWK::Graphics::FBXModelLoaderBase::FetchWorldVertexTangent(const ufbx_node* a_fbxNode, const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxNode, "ufbx_nodeがnullptrのため、ワールド頂点接線の取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "ufbx_meshがnullptrのため、ワールド頂点接線の取得に失敗しました。", {});

	ufbx_vec3 l_localTangent = { k_defaultTangentX, k_defaultTangentY, k_defaultTangentZ };

	if (a_fbxMesh->vertex_tangent.exists)
	{
		l_localTangent = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_tangent, a_vertexIndex);
		l_localTangent = ufbx_vec3_normalize (l_localTangent);
	}

	// Tangentは方向ベクトルなので、位置ではなくdirectionとして変換する
	auto l_worldTangent = ufbx_transform_direction(&a_fbxNode->geometry_to_world, l_localTangent);

	l_worldTangent = ufbx_vec3_normalize(l_worldTangent);

	return
	{
		static_cast<float>(l_worldTangent.x),
		static_cast<float>(l_worldTangent.y),
		static_cast<float>(l_worldTangent.z),
		k_defaultTangentW
	};
}

std::wstring FWK::Graphics::FBXModelLoaderBase::FetchMaterialTextureFilePath(const ufbx_material_map& a_materialMap) const
{
	const auto* l_fbxTexture = a_materialMap.texture;

	if (!l_fbxTexture) { return {}; }

	std::filesystem::path l_textureFilePath = {};

	if (l_fbxTexture->type != UFBX_TEXTURE_FILE) { return {}; }

	// ufbx_texture_typeがUFBX_TEXTURE_FILEの場合、
	// filename / relative_filenameに画像ファイルパスが入っている
	if (l_fbxTexture->relative_filename.length != Constant::k_emptyStringLength)
	{
		l_textureFilePath = Utility::ConvertUFBXStringToWString(l_fbxTexture->relative_filename);
	}
	else if (l_fbxTexture->filename.length != Constant::k_emptyStringLength)
	{
		l_textureFilePath = Utility::ConvertUFBXStringToWString(l_fbxTexture->filename);
	}

	if (l_textureFilePath.empty()) { return {}; }

	// 現在のTextureSystemはPNG読み込み方針なので、
	// FBX内のpng等の参照をエンジンで使うpngパスへ変換する
	l_textureFilePath.replace_extension(Constant::k_lowerPNGExtension);

	return l_textureFilePath.wstring();
}

FWK::TypeAlias::Math::Color FWK::Graphics::FBXModelLoaderBase::FetchBaseColorFactor(const ufbx_material_map& a_materialMap) const
{
	if (!a_materialMap.has_value) { return Struct::ModelMaterialAssetData::k_defaultModelMaterialBaseColorFactor; }

	return TypeAlias::Math::Color
	{
		static_cast<float>(a_materialMap.value_vec4.x),
		static_cast<float>(a_materialMap.value_vec4.y),
		static_cast<float>(a_materialMap.value_vec4.z),
		static_cast<float>(a_materialMap.value_vec4.w)
	};
}

float FWK::Graphics::FBXModelLoaderBase::FetchMaterialFactor(const ufbx_material_map& a_materialMap, const float a_defaultValue) const
{
	if (!a_materialMap.has_value) { return a_defaultValue; }

	return static_cast<float>(a_materialMap.value_real);
}

ufbx_load_opts FWK::Graphics::FBXModelLoaderBase::CreateFBXLoadOptions() const
{
	// ufbx_load_optsは、ufbxでFBXを読み込むときの設定
	// 座標系変換とスケール補正は自作エンジン側で一貫して行うため、
	// ufbxのroot_transformでは変換しない
	ufbx_load_opts l_loadOptions = {};

	// generate_missing_normals;
	// trueにすると、FBX内に頂点法線が存在しないMeshに対してufbx側で法線を生成する
	// Lit / NormalMapのライティングでは法線が必要なので、欠けている場合は読み込み時に補完する
	l_loadOptions.generate_missing_normals = true;

	// normalize_normals;
	// trueにすると、読み込んだ頂点法線を正規化する
	// ライティング計算では長さ1の法線を前提にするため、読み込み時点で正規化しておく
	l_loadOptions.normalize_normals = true;

	// normalize_tangetns
	// これを true にしておくと、NormalMapを使ったPBRライティングで
	// 接線の長さが原因のライティング崩れを防ぎやすくなる。
	l_loadOptions.normalize_tangents = true;

	// 単位変換
	// 変換後は1.0 = 1mとして扱う
	// 元FBXがcm単位なら100cm->1.0mになる
	l_loadOptions.target_unit_meters = k_modelFBXTargetUnitMeters;

	// target_axes/target_unit_metersの変換方法
	// MODIFY_GEOMETRYにすると座標系変換・単位変換をGeometryにも反映する
	l_loadOptions.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;

	// FBXのGeometryTransformもGeometry側へ反映する
	// これにより、FBX特有の「Geometryだけに聞くTransform」を扱いやすくする
	l_loadOptions.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_MODIFY_GEOMETRY;

	return l_loadOptions;
}