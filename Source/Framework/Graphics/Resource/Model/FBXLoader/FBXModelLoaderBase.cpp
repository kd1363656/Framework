#include "FBXModelLoaderBase.h"

ufbx_scene* FWK::Graphics::FBXModelLoaderBase::LoadFBXScene(const std::filesystem::path& a_filePath) const
{
	// FBXファイルとして読み込めるパスか確認する
	// 存在しないファイルや.fbx以外のファイルをufbxへ渡さないための事前チェック
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "FBXファイルが読み込める形式ではありません、FBXシーンの読み込みに失敗しました。", nullptr);

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

void FWK::Graphics::FBXModelLoaderBase::DestroyFBXScene(ufbx_scene* a_fbxScene) const
{
	if (!a_fbxScene) { return; }

	// ufbx_load_file()で作成されたufbx_sceneは、使い終わったらufbx_free_scene()で解放する
	// 今回はModelDataへ必要な情報をコピーした後、StaticModelFBXLoader側でこの関数を呼ぶ
	ufbx_free_scene(a_fbxScene);
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::FetchLocalVertexPosition(const ufbx_mesh * a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxMesh, "ufbx_meshがnullptrのため、ローカル頂点座標の取得に失敗しました。", {});

	// ufbx_get_vertex_vec3()で、指定した頂点インデックスの座標を取得する
	// ufbx_mesh::vertex_positionには、FBX内のローカル空間の頂点座標データが入っている
	const auto& l_position = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_position, a_vertexIndex);

	return ConvertUFBXVector3ToVector3(l_position);
}
FWK::TypeAlias::Math::Vector2 FWK::Graphics::FBXModelLoaderBase::FetchVertexUV(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxMesh, "ufbx_meshがnullptrのため、UVの取得に失敗しました。", {});

	// FBXによってはUVが入っていない場合がある
	// その場合は今は空のVector2を返し、読み込み自体は続行する
	if (!a_fbxMesh->vertex_uv.exists) { return {}; }

	// ufbx_get_vertex_vec2()で、指定した頂点インデックスのUVを取得する
	// ufbx_mesh::vertex_uvには、FBX内のUV座標データが入っている
	const auto& l_uv = ufbx_get_vertex_vec2(&a_fbxMesh->vertex_uv, a_vertexIndex);

	auto l_convertedUV = ConvertUFBXVector2ToVector2(l_uv);

	// DirectXのUV座標に合わせるため、V座標を反転する
	// BlenderなどのDCCツールとDirectXでは、テクスチャの上下方向の扱いが異なる場合がある
	l_convertedUV.y = k_uvCoordinateMax - l_convertedUV.y;

	return l_convertedUV;
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::FetchLocalVertexNormal(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxMesh, "ufbx_meshがnullptrのため、ローカル頂点法線の取得に失敗しました。", {});

	if (!a_fbxMesh->vertex_normal.exists) { return {}; }

	// ufbx_get_vertex_vec3(取得する頂点属性、
	//						取得したい頂点Index);
	const auto& l_normal = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_normal, a_vertexIndex);

	// ufbx_vec3_normalize(正規化したいベクトル);
	// ライティング計算では長さ1の法線を前提にするため、取得時点で正規化する
	const auto& l_normalizedNormal = ufbx_vec3_normalize(l_normal);

	return ConvertUFBXVector3ToVector3(l_normalizedNormal);
}
FWK::TypeAlias::Math::Vector4 FWK::Graphics::FBXModelLoaderBase::FetchLocalVertexTangent(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_fbxMesh, "ufbx_meshがnullptrのため、頂点接線の取得に失敗しました。", {});

	if (!a_fbxMesh->vertex_tangent.exists)
	{
		return 
		{
			k_defaultTangentX,
			k_defaultTangentY,
			k_defaultTangentZ,
			k_defaultTangentW
		};
	}

	// ufbx_get_vertex_vec3(取得する頂点属性、
	//						取得したい頂点Index);
	const auto& l_tangent = ufbx_get_vertex_vec3(&a_fbxMesh->vertex_tangent, a_vertexIndex);

	// ufbx_vec3_normalize(正規化したいベクトル);
	// NormalMapのTBN計算では長さ1の接線を前提にするため、取得時点で正規化する
	const auto& l_normalizedTangent = ufbx_vec3_normalize(l_tangent);

	return
	{
		static_cast<float>(l_normalizedTangent.x),
		static_cast<float>(l_normalizedTangent.y),
		static_cast<float>(l_normalizedTangent.z),
		k_defaultTangentW
	};
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::TransformImportPosition(const TypeAlias::Math::Vector3& a_position) const
{
	// UnrealEngineで作成・出力したモデルをBlenderへ持ち込み、
	// そのBlenderからFBXとして出力したモデルを読み込む前提で座標を補正する前提である。
	// FBX/Blender側の座標と、自作エンジン側の座標系では、
	// 「上方向」や「下方向」として扱う軸が異なるため、
	// 読み込んだ頂点座標をそのまま使うとモデルの向きが合わない、
	// そのため、ここでは読み込んだ座標を以下の通りに並び替える
	// 読み込み元 : x,y,z -> 自作エンジン : x,z,-y
	// yとzを入れ替えることで、Blender/FBX側の上方向を自作エンジン側の上方向へ合わせる。
	// さらに-yにすることで前方向の向きを自作エンジンの左手系座標へ合わせる
	// また、UnrealEngineは標準でcm単位を使うため、0.01倍にしてcm単位からm単位へ変換する
	return
	{
		a_position.x * k_modelImportScale,
		a_position.z * k_modelImportScale,
		a_position.y * k_importAxisNegative * k_modelImportScale
	};
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::TransformImportNormal(const TypeAlias::Math::Vector3& a_normal) const
{
	// UnrealEngineで作成・出力したモデルをBlenderへ持ち込み、
	// そのBlenderからFBXとして出力したモデルを読み込む前提で座標を補正する前提である。
	// FBX/Blender側の座標と、自作エンジン側の座標系では、
	// 「上方向」や「下方向」として扱う軸が異なるため、
	// 読み込んだ頂点座標をそのまま使うとモデルの向きが合わない、
	// そのため、ここでは読み込んだ座標を以下の通りに並び替える
	// 読み込み元 : x,y,z -> 自作エンジン : x,z,-y
	// yとzを入れ替えることで、Blender/FBX側の上方向を自作エンジン側の上方向へ合わせる。
	// 法線は方向ベクトルなので、位置のような平行移動やスケール補正は使わない
	TypeAlias::Math::Vector3 l_normal =
	{
		a_normal.x,
		a_normal.z,
		a_normal.y * k_importAxisNegative
	};

	l_normal.Normalize();

	return l_normal;
}
FWK::TypeAlias::Math::Vector4 FWK::Graphics::FBXModelLoaderBase::TransformImportTangent(const TypeAlias::Math::Vector4& a_tangent) const
{
	// UnrealEngineで作成・出力したモデルをBlenderへ持ち込み、
	// そのBlenderからFBXとして出力したモデルを読み込む前提で座標を補正する前提である。
	// FBX/Blender側の座標と、自作エンジン側の座標系では、
	// 「上方向」や「下方向」として扱う軸が異なるため、
	// 読み込んだ頂点座標をそのまま使うとモデルの向きが合わない、
	// そのため、ここでは読み込んだ座標を以下の通りに並び替える
	// 読み込み元 : x,y,z -> 自作エンジン : x,z,-y
	// yとzを入れ替えることで、Blender/FBX側の上方向を自作エンジン側の上方向へ合わせる。
	// 接線は方向ベクトルなので、位置のような平行移動やスケール補正は使わない
	// xyzは軸変換後に正規化し、wはTangent空間の向き判定に使うため維持する
	TypeAlias::Math::Vector3 l_tangent =
	{
		a_tangent.x,
		a_tangent.z,
		a_tangent.y * k_importAxisNegative
	};

	l_tangent.Normalize();

	return
	{
		l_tangent.x,
		l_tangent.y,
		l_tangent.z,
		a_tangent.w,
	};
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::FBXModelLoaderBase::ConvertUFBXVector3ToVector3(const ufbx_vec3& a_fbxVector) const
{
	// ufbx_vec3はdouble系の値を持つため、自作フレームワークのVector3で使うfloatへ変換する
	return TypeAlias::Math::Vector3
	(
		static_cast<float>(a_fbxVector.x),
		static_cast<float>(a_fbxVector.y),
		static_cast<float>(a_fbxVector.z)
	);
}
FWK::TypeAlias::Math::Vector2 FWK::Graphics::FBXModelLoaderBase::ConvertUFBXVector2ToVector2(const ufbx_vec2& a_fbxVector) const
{
	// ufbx_vec2はdouble系の値を持つため、自作フレームワークのVector2で使うfloatへ変換する
	return TypeAlias::Math::Vector2
	(
		static_cast<float>(a_fbxVector.x),
		static_cast<float>(a_fbxVector.y)
	);
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

	// use_root_transform;
	// trueにするとufbx側でroot_transformが適用される
	// 今回は自作エンジン側で頂点 / 法線 / 接線 / Node / Boneを同じ変換規則で揃えるためfalseにする
	l_loadOptions.use_root_transform = false;

	return l_loadOptions;
}