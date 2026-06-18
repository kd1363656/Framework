#include "StaticModelMeshletBuilder.h"

bool FWK::Graphics::StaticModelMeshletBuilder::BuildStaticModelRecordMeshletData(Graphics::StaticModelRecord& a_staticModelRecord) const
{
	for (auto& l_modelMesh : a_staticModelRecord.GetMutableREFModelData().m_modelMeshList)
	{
		// StaticModelRecord内の全StaticModelMeshに対して、
		// MeshShader用のStaticModelMeshletDataを作成する
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!BuildModelMeshletData(l_modelMesh), "StaticModelMeshのStaticModelMeshletData作成に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::StaticModelMeshletBuilder::BuildModelMeshletData(Struct::StaticModelMesh& a_staticModelMesh) const
{
	// 頂点数とインデックス数のチェック
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_staticModelMesh.m_modelVertexList.size() == Constant::k_emptyModelVertexCount, "StaticModelMeshの頂点数が0のため、MeshletData作成に失敗しました。",			false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_staticModelMesh.m_indexList.size()		 == Constant::k_emptyModelIndexCount,  "StaticModelMeshのインデックス数が0のため、MeshletData作成に失敗しました。", false);

	// インデックスリストの総数を3で割った時に余りが0でないと、三角形を構成するインデックスリストとして不適切
	FWK_ASSERT_RETURN_VALUE_IF_FAILED((a_staticModelMesh.m_indexList.size() % Constant::k_triangleVertexCount) != k_emptyRemainder, "StaticModelMeshのインデックス数が三角形単位ではないため、StaticModelMeshletData作成に失敗しました。", false);

	// メッシュレットデータの初期化
	// 前回読み込んでいたモデルのメッシュレット情報が残るのを防ぐため
	auto& l_modelMeshletData = a_staticModelMesh.m_modelMeshletData;

	l_modelMeshletData.m_meshletList.clear          ();
	l_modelMeshletData.m_uniqueVertexIndexList.clear();
	l_modelMeshletData.m_primitiveIndexList.clear   ();
	l_modelMeshletData.m_meshletBoundsList.clear    ();

	// Meshlet作成に必要になる最大Meshlet数を取得する
	// meshopt_buildMeshletsBound(インデックス数、
	//						      Meshlet内の最大頂点数、
	//							  Meshlet内の最大三角形数);
	const auto l_maxMeshletCount = meshopt_buildMeshletsBound(a_staticModelMesh.m_indexList.size(), Constant::k_maxMeshletVertexCount, Constant::k_maxMeshletPrimitiveCount);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_maxMeshletCount == Constant::k_emptyMeshletCount, "Meshletの最大数が0のため、StaticModelMeshletData作成に失敗しました。", false);

	std::vector<meshopt_Meshlet> l_meshoptMeshletList		 = {};
	std::vector<uint8_t>		 l_meshoptPrimitiveIndexList = {};

	// メッシュレット情報を最大Meshlet数分確保する
	l_meshoptMeshletList.resize(l_maxMeshletCount);

	// 一つのMeshletが参照できる頂点数は最大64個、
	// そのため、最大Meshlet数 * 1Meshlet辺りの最大頂点数分の
	// 「元モデル頂点インデックス」を格納できるように確保する
	l_modelMeshletData.m_uniqueVertexIndexList.resize(l_maxMeshletCount * Constant::k_maxMeshletVertexCount);

	// 1つのMeshletが持てる三角形数は最大128個。
	// 三角形1つにつき頂点インデックスが3個必要なので、
	// 最大Meshlet数 * 1Meshlet辺りの最大三角形数 * 3個分を確保する
	l_meshoptPrimitiveIndexList.resize(l_maxMeshletCount * Constant::k_maxMeshletPrimitiveCount * Constant::k_triangleVertexCount);

	// meshoptimizerは頂点座標をfloat*とstrideで受け取る
	// reinterpret_castでModelVertex全体をfloat*に見せるより、
	// 先頭頂点のm_position.xを直接渡す方が安全
	const auto* l_vertexPositionData = &a_staticModelMesh.m_modelVertexList.front().m_position.x;

	// 最適化済みの頂点とインデックスからMeshShader用のStaticModelMeshletDataを作成する
	// meshopt_buildMeshlets(出力Meshlet配列、
	//						 出力PrimitiveIndex配列、
	//					     入力Index配列、
	//						 入力Index数、
	//						 入力頂点座標配列、
	//						 入力頂点数、
	//						 入力頂点1個分のbyteサイズ、
	//						 Meshlet内の最大頂点数、
	//						 Meshlet内の最大三角形数、
	//						 ConeCulling用の重み);
	const auto l_meshletCount = meshopt_buildMeshlets(l_meshoptMeshletList.data(),
													  l_modelMeshletData.m_uniqueVertexIndexList.data(),
													  l_meshoptPrimitiveIndexList.data(),
													  a_staticModelMesh.m_indexList.data(),
													  a_staticModelMesh.m_indexList.size(),
													  l_vertexPositionData,
													  a_staticModelMesh.m_modelVertexList.size(),
													  sizeof(Struct::StaticModelVertex),
													  Constant::k_maxMeshletVertexCount,
													  Constant::k_maxMeshletPrimitiveCount,
													  k_defaultMeshletConeWeight);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_meshletCount == Constant::k_emptyMeshletCount, "MeshletData作成結果のMeshlet数が0です。", false);

	// メッシュレットに必要な分のみ要素を確保
	l_meshoptMeshletList.resize(l_meshletCount);

	const auto& l_lastMeshlet = l_meshoptMeshletList.back();

	// 頂点インデックスリストに必要な分のみ要素を確保(最後のメッシュレットのオフセットと頂点数を足した数が必要なサイズ)
	l_modelMeshletData.m_uniqueVertexIndexList.resize(l_lastMeshlet.vertex_offset + l_lastMeshlet.vertex_count);

	const auto l_lastMeshletPrimitiveIndexCount = l_lastMeshlet.triangle_count * Constant::k_triangleVertexCount;

	// meshoptimizerのPrimitiveIndex配列は4byte境界にそろえて扱うため、
	// 最後のMeshletで実際に使用したPrimitiveIndex数を4byte境界へ切り上げる
	const auto& l_alignedLastMeshletPrimitiveIndexCount = Utility::AlignUp(l_lastMeshletPrimitiveIndexCount, k_meshletPrimitiveIndexAlignment);

	const auto& l_usedPrimitiveIndexCount = l_lastMeshlet.triangle_offset + l_alignedLastMeshletPrimitiveIndexCount;

	// プリミティブインデックスリストに必要な分のみ要素を確保
	l_meshoptPrimitiveIndexList.resize            (l_usedPrimitiveIndexCount);
	l_modelMeshletData.m_primitiveIndexList.resize(l_usedPrimitiveIndexCount);

	// meshoptimizerが出力したPrimitiveIndexListを、
	// プロジェクト側で保存するMeshletDataのPrimitiveIndexListへコピーする
	for (size_t l_primitiveIndex = 0ULL; l_primitiveIndex < l_usedPrimitiveIndexCount; ++l_primitiveIndex)
	{
		l_modelMeshletData.m_primitiveIndexList[l_primitiveIndex] = static_cast<std::uint32_t>(l_meshoptPrimitiveIndexList[l_primitiveIndex]);
	}

	// 最後にメッシュレット数とメッシュレットカリング用リストのリサイズを行い、オーバーヘッドが出ないようにする
	l_modelMeshletData.m_meshletList.resize		 (l_meshletCount);
	l_modelMeshletData.m_meshletBoundsList.resize(l_meshletCount);

	// メッシュレットの頂点情報、インデックス情報取得用パラメータやカリング用パラメータを格納
	for (std::size_t l_meshletIndex = 0ULL; l_meshletIndex < l_meshletCount; ++l_meshletIndex)
	{
		const auto& l_meshoptMeshlet = l_meshoptMeshletList[l_meshletIndex];

		auto& l_modelMeshlet = l_modelMeshletData.m_meshletList[l_meshletIndex];

		// 頂点情報を格納
		l_modelMeshlet.m_vertexOffset   = l_meshoptMeshlet.vertex_offset;
		l_modelMeshlet.m_triangleOffset = l_meshoptMeshlet.triangle_offset;
		l_modelMeshlet.m_vertexCount    = l_meshoptMeshlet.vertex_count;
		l_modelMeshlet.m_triangleCount  = l_meshoptMeshlet.triangle_count;

		// Meshlet単位のFrustum CullingやBackfaceConeCullingに使用する境界情報を作成する
		// meshopt_computeMeshletBounds(入力UniqueVertexIndex、
		//								入力PrimitiveIndex配列、
		//								Meshlet内の三角形数、
		//								入力頂点座標配列、
		//								入力頂点数、
		//								入力頂点1個分のbyteサイズ);
		const auto l_meshoptBounds = meshopt_computeMeshletBounds(l_modelMeshletData.m_uniqueVertexIndexList.data() + l_meshoptMeshlet.vertex_offset,
																  l_meshoptPrimitiveIndexList.data()				+ l_meshoptMeshlet.triangle_offset,
																  l_meshoptMeshlet.triangle_count,
																  l_vertexPositionData,
																  a_staticModelMesh.m_modelVertexList.size(),
																  sizeof(Struct::StaticModelVertex));

		// メッシュレットカリング用情報を格納
		auto& l_modelMeshletBounds = l_modelMeshletData.m_meshletBoundsList[l_meshletIndex];

		// Meshletを囲むBoundingSphereの中心座標。
		// 個の中心座標と半径を使うことで、Meshlet単位でFrustumCullingを行える
		l_modelMeshletBounds.m_center.x = l_meshoptBounds.center[k_vectorElementIndexX];
		l_modelMeshletBounds.m_center.y = l_meshoptBounds.center[k_vectorElementIndexY];
		l_modelMeshletBounds.m_center.z = l_meshoptBounds.center[k_vectorElementIndexZ];

		// Meshletを囲むBoundingSphereの半径。
		// m_centerを中心とした球の半径
		// FrustumCullingではcenter + radiusを使って、Meshletがカメラに映る可能性があるかを判定する
		l_modelMeshletBounds.m_radius   = l_meshoptBounds.radius;
		
		// BackfaceConeCulling用のConeの頂点位置
		// Meshlet内の三角形群が作る「おおよその向き」をConeとして表すときの起点。
		// ConeCullingではCone情報を使って、Meshlet全体がカメラから見てどうかを判定する
		l_modelMeshletBounds.m_coneApex.x = l_meshoptBounds.cone_apex[k_vectorElementIndexX];
		l_modelMeshletBounds.m_coneApex.y = l_meshoptBounds.cone_apex[k_vectorElementIndexY];
		l_modelMeshletBounds.m_coneApex.z = l_meshoptBounds.cone_apex[k_vectorElementIndexZ];

		// BackfaceConeCulling用のしきい値
		// coneAxisと視線方向の内積を使って、このMeshletがカメラから見て裏向きかどうかを判定するための値
		// この値を使うことで、Meshlet内の三角形が全て裏向きと判断できる場合に、
		// MeshShaderやPixelShaderの処理を省略できる
		l_modelMeshletBounds.m_coneCutoff = l_meshoptBounds.cone_cutoff;

		// Meshlet内の三角形群が、おおよそどちらを向いているかを表す方向ベクトル
		// カメラ方向とこの方向を比較して、Meshlet全体が裏向きなら描画をスキップするために使う
		// Normal単体ではなく、Meshlet内の複数三角形の向きをまとめた代表方向
		l_modelMeshletBounds.m_coneAxis.x = l_meshoptBounds.cone_axis[k_vectorElementIndexX];
		l_modelMeshletBounds.m_coneAxis.y = l_meshoptBounds.cone_axis[k_vectorElementIndexY];
		l_modelMeshletBounds.m_coneAxis.z = l_meshoptBounds.cone_axis[k_vectorElementIndexZ];
	}

	return true;
}