#include "StaticModelMeshOptimizer.h"

bool FWK::Graphics::StaticModelMeshOptimizer::OptimizeStaticModelRecord(Graphics::StaticModelRecord& a_staticModelRecord) const
{
	auto& l_modelData = a_staticModelRecord.GetMutableREFModelData();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_modelData.m_modelMeshList.empty(), "ModelDataのMeshリストが空のため、StaticModelMeshの最適化に失敗しました。", false);

	for (auto& l_modelMesh : l_modelData.m_modelMeshList)
	{
		// Mesh単位で最適化する
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!OptimizeStaticModelMesh(l_modelMesh), "ModelMeshの最適化に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::StaticModelMeshOptimizer::OptimizeStaticModelMesh(Struct::StaticModelMesh& a_staticModelMesh) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_staticModelMesh.m_modelVertexList.empty(), "ModelMeshの頂点リストが空のため、StaticModelMeshの最適化に失敗しました。",		   false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_staticModelMesh.m_indexList.empty(),		   "ModelMeshのインデックスリストが空のため、StaticModelMeshの最適化に失敗しました。", false);

	std::vector<std::uint32_t> l_vertexRemapList = {};

	l_vertexRemapList.resize(a_staticModelMesh.m_modelVertexList.size());

	// 同じposition / uvを持つ頂点をまとめるための対応表を作成する
	// meshopt_generateVertexRemap(古い頂点番号から新しい頂点番号への対応表を書き込む配列、
	//						       現在のインデックス配列、	
	//							   現在のインデックス数、
	//							   現在の頂点配列、	
	//							   現在の頂点数、
	//						       頂点一つ分のバイトサイズ);
	const auto& l_optimizedVertexCount = meshopt_generateVertexRemap(l_vertexRemapList.data(),
																	 a_staticModelMesh.m_indexList.data(),
																	 a_staticModelMesh.m_indexList.size(),
																	 a_staticModelMesh.m_modelVertexList.data(),
																	 a_staticModelMesh.m_modelVertexList.size(),
																	 sizeof(Struct::StaticModelVertex));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_optimizedVertexCount == k_invalidOptimizedVertexCount, "meshopt_generateVertexRemapによる頂点リマップ作成に失敗しました。", false);

	std::vector<std::uint32_t> l_optimizedIndexList = {};

	l_optimizedIndexList.resize(a_staticModelMesh.m_indexList.size());

	// 古い頂点番号を、重複削除後の新しい頂点番号へ変換する
	// meshopt_remapIndexBuffer(最適化後のインデックス配列の書き込み先、
	//							現在のインデックス配列、
	//							現在のインデックス数、
	//							meshopt_generateVertexRemapで作成した対応表);
	meshopt_remapIndexBuffer(l_optimizedIndexList.data(),
							 a_staticModelMesh.m_indexList.data(),
							 a_staticModelMesh.m_indexList.size(),
							 l_vertexRemapList.data());

	std::vector<Struct::StaticModelVertex> l_optimizedModelVertexList = {};

	l_optimizedModelVertexList.resize(l_optimizedVertexCount);

	// 重複している頂点を削除し、必要な頂点だけを新しい頂点配列へ詰めなおす
	// meshopt_remapVertexBuffer(最適化後の頂点配列の書き込み先、
	//							 現在の頂点配列、
	//							 現在の頂点数、
	//							 頂点一つ分のバイトサイズ、
	//							 meshopt_generateVertexRemapで作成した対応表);
	meshopt_remapVertexBuffer(l_optimizedModelVertexList.data(),
							  a_staticModelMesh.m_modelVertexList.data(),
							  a_staticModelMesh.m_modelVertexList.size(),
							  sizeof(Struct::StaticModelVertex),
							  l_vertexRemapList.data());

	// GPUの頂点キャッシュに乗りやすい順番へインデックスを並べ替える
	// meshopt_optimizeVertexCache(最適化後のインデックス配列の書き込み先、
	//							   現在のインデックス配列、
	//							   現在のインデックス、
	//							   現在の頂点数);
	meshopt_optimizeVertexCache(l_optimizedIndexList.data(),
								l_optimizedIndexList.data(),
								l_optimizedIndexList.size(),
								l_optimizedModelVertexList.size());

	// インデックスから参照される順番に頂点を並び替え、GPUが頂点を読みやすい配置にする
	// meshopt_optimizeVertexFetch(最適化後の頂点配列の書き込み先、
	//							   最適化後のインデックス配列、
	//							   インデックス数、
	//							   現在の頂点配列、
	//							   頂点一つ分のバイトサイズ);
	meshopt_optimizeVertexFetch(l_optimizedModelVertexList.data(),
								l_optimizedIndexList.data(),
								l_optimizedIndexList.size(),
								l_optimizedModelVertexList.data(),
								l_optimizedModelVertexList.size(),
								sizeof(Struct::StaticModelVertex));

	// 最適化後のインデックス、頂点のリストを格納
	a_staticModelMesh.m_modelVertexList = std::move(l_optimizedModelVertexList);
	a_staticModelMesh.m_indexList       = std::move(l_optimizedIndexList);

	return true;
}