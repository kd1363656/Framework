#include "StaticModelMeshOptimizer.h"

bool FWK::Graphics::StaticModelMeshOptimizer::OptimizeStaticModelRecord(Graphics::StaticModelRecord& a_staticModelRecord) const
{
	auto& l_modelData = a_staticModelRecord.GetMutableREFModelData();

	FWK_ASSERT_RETURN_VALUE_IF(l_modelData.m_modelMeshList.empty(), "ModelDataのMeshリストが空のため、StaticModelMeshの最適化に失敗しました。", false);

	for (auto& l_modelMesh : l_modelData.m_modelMeshList)
	{
		// StaticModelDataはMaterial単位などで複数のStaticModelMeshを持つ。
		// meshoptimizerはMesh単位のVertexList/IndexListに対して適用するため、
		// StaticModelMeshごとに個別に最適化する
		FWK_ASSERT_RETURN_VALUE_IF(!OptimizeStaticModelMesh(l_modelMesh), "ModelMeshの最適化に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::StaticModelMeshOptimizer::OptimizeStaticModelMesh(StaticModelRecord::StaticModelMesh& a_staticModelMesh) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_staticModelMesh.m_modelVertexList.empty(), "ModelMeshの頂点リストが空のため、StaticModelMeshの最適化に失敗しました。",		    false);
	FWK_ASSERT_RETURN_VALUE_IF(a_staticModelMesh.m_indexList.empty(),		"ModelMeshのインデックスリストが空のため、StaticModelMeshの最適化に失敗しました。", false);

	std::vector<std::uint32_t> l_vertexRemapList = {};

	// Remap表は「古い頂点番号 -> 新しい頂点番号」の対応表。
	// そのため、Index数ではなく、元の頂点数と同じ要素数を確保する。
	l_vertexRemapList.resize(a_staticModelMesh.m_modelVertexList.size());

	// 重複している頂点をまとめるためのRemap表を作成する。
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
																	 sizeof(StaticModelRecord::StaticModelVertex));

	FWK_ASSERT_RETURN_VALUE_IF(l_optimizedVertexCount == k_invalidOptimizedVertexCount, "meshopt_generateVertexRemapによる頂点リマップ作成に失敗しました。", false);

	std::vector<std::uint32_t> l_optimizedIndexList = {};

	l_optimizedIndexList.resize(a_staticModelMesh.m_indexList.size());

	// 現在のIndexListを、Remap表に従って新しい頂点番号へ変換する
	// meshopt_remapIndexBuffer(最適化後のインデックス配列の書き込み先、
	//							現在のインデックス配列、
	//							現在のインデックス数、
	//							meshopt_generateVertexRemapで作成した対応表);
	meshopt_remapIndexBuffer(l_optimizedIndexList.data(),
							 a_staticModelMesh.m_indexList.data(),
							 a_staticModelMesh.m_indexList.size(),
							 l_vertexRemapList.data());

	std::vector<StaticModelRecord::StaticModelVertex> l_optimizedModelVertexList = {};

	l_optimizedModelVertexList.resize(l_optimizedVertexCount);

	// Remap表に従って、重複を取り除いた新しいVertexListを作成する
	// meshopt_remapVertexBuffer(最適化後の頂点配列の書き込み先、
	//							 現在の頂点配列、
	//							 現在の頂点数、
	//							 頂点一つ分のバイトサイズ、
	//							 meshopt_generateVertexRemapで作成した対応表);
	meshopt_remapVertexBuffer(l_optimizedModelVertexList.data(),
							  a_staticModelMesh.m_modelVertexList.data(),
							  a_staticModelMesh.m_modelVertexList.size(),
							  sizeof(StaticModelRecord::StaticModelVertex),
							  l_vertexRemapList.data());

	// GPUの頂点キャッシュに乗りやすいように、IndexListの順番を並べ替える
	// meshopt_optimizeVertexCache(最適化後のインデックス配列の書き込み先、
	//							   現在のインデックス配列、
	//							   現在のインデックス、
	//							   現在の頂点数);
	meshopt_optimizeVertexCache(l_optimizedIndexList.data(),
								l_optimizedIndexList.data(),
								l_optimizedIndexList.size(),
								l_optimizedModelVertexList.size());

	// IndexListから参照される順番に合わせて、VertexListのメモリ配置を並び替える。
	// meshopt_optimizeVertexFetch(最適化後の頂点配列の書き込み先、
	//							   最適化後のインデックス配列、
	//							   インデックス数、
	//							   現在の頂点配列、
	//							   現在の頂点数、
	//							   頂点一つ分のバイトサイズ);
	meshopt_optimizeVertexFetch(l_optimizedModelVertexList.data(),
								l_optimizedIndexList.data(),
								l_optimizedIndexList.size(),
								l_optimizedModelVertexList.data(),
								l_optimizedModelVertexList.size(),
								sizeof(StaticModelRecord::StaticModelVertex));

	// 最適化後のインデックス、頂点のリストを格納
	a_staticModelMesh.m_modelVertexList = std::move(l_optimizedModelVertexList);
	a_staticModelMesh.m_indexList       = std::move(l_optimizedIndexList);

	return true;
}