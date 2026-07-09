#include "PhysicsDebugTriangleBatch.h"

JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugTriangleBatch::Create(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount)
{
	// JoltのBatchはJPH::Refによる侵入型参照カウントを使用する
	// newで生成した直後にBatchへ渡すため、生のポインタとして保持はしない
	auto l_batch = JPH::DebugRenderer::Batch(new PhysicsDebugTriangleBatch());

	// 今、作成したバッチの内部にあるRefTargetVirtualをPhysicsDebugTriangleBatchにキャスト
	auto& l_batchImplementation = static_cast<PhysicsDebugTriangleBatch&>(*l_batch.GetPtr());

	// 三角形単位にデータが完成しているものをそのままコピーする
	l_batchImplementation.CopyTriangleList(a_triangleList, a_triangleCount);

	return l_batch;
}

JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugTriangleBatch::Create(const JPH::DebugRenderer::Vertex* a_vertexList, 
																		  const int						    a_vertexCount,
																		  const JPH::uint32*				a_indexList, 
																		  const int					        a_indexCount)
{
	auto l_batch = JPH::DebugRenderer::Batch(new PhysicsDebugTriangleBatch());

	auto& l_batchImplementation = static_cast<PhysicsDebugTriangleBatch&>(*l_batch.GetPtr());

	l_batchImplementation.CopyIndexedTriangleList(a_vertexList, 
												  a_vertexCount,	
												  a_indexList,
												  a_indexCount);

	return l_batch;
}

void FWK::Physics::PhysicsDebugTriangleBatch::AddRef()
{
	++m_referenceCount;
}

void FWK::Physics::PhysicsDebugTriangleBatch::Release()
{
	// 参照カウントが0になればこのクラスを削除する
	if (const auto l_referenceCount = --m_referenceCount;
		l_referenceCount != k_noReferenceCount)
	{
		return; 
	}

	// 解放時に自身をdelete
	delete this;
}

void FWK::Physics::PhysicsDebugTriangleBatch::CopyTriangleList(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount)
{
	m_triangleList.clear();

	if (!a_triangleList || 
		a_triangleCount <= k_invalidTriangleCount)
	{
		return; 
	}

	// 三角形情報をコピー
	m_triangleList.assign(a_triangleList, a_triangleList + a_triangleCount);
}

void FWK::Physics::PhysicsDebugTriangleBatch::CopyIndexedTriangleList(const JPH::DebugRenderer::Vertex* a_vertexList,
																	  const int						    a_vertexCount,
																	  const JPH::uint32*				a_indexList,
																	  const int						    a_indexCount)
{
	m_triangleList.clear();

	if (!a_vertexList							||
		a_vertexCount <= k_invalidTriangleCount ||
		!a_indexList							||
		a_indexCount <= k_invalidIndexCount)
	{
		return;
	}

	// 3で割った時に余りが0出ないと三角形を構成できるインデックス数ではないためassert
	FWK_ASSERT_RETURN_IF(a_indexCount % Converter::StaticModelBinaryConverter::k_triangleVertexCount != k_emptyRemainder, "PhysicsDebug用Index数が三角形を構成できない値です。");

	const auto l_triangleCount = static_cast<std::size_t>(a_indexCount) / static_cast<std::size_t>(Converter::StaticModelBinaryConverter::k_triangleVertexCount);

	// 必要な三角形数を一度だけ作成する
	m_triangleList.resize(l_triangleCount);
	
	for (std::size_t l_triangleIndex = 0ULL; l_triangleIndex < l_triangleCount; ++l_triangleIndex)
	{
		const auto l_baseIndex = l_triangleIndex * static_cast<std::size_t>(Converter::StaticModelBinaryConverter::k_triangleVertexCount);

		FWK_ASSERT_RETURN_IF(a_indexList[l_baseIndex] + k_vertexZeroOffset >= static_cast<JPH::uint32>(a_vertexCount), "PhysicsDebug用IndexがVertex数の範囲を超えています。");
		FWK_ASSERT_RETURN_IF(a_indexList[l_baseIndex] + k_vertexOneOffset  >= static_cast<JPH::uint32>(a_vertexCount), "PhysicsDebug用IndexがVertex数の範囲を超えています。");
		FWK_ASSERT_RETURN_IF(a_indexList[l_baseIndex] + k_vertexTwoOffset  >= static_cast<JPH::uint32>(a_vertexCount), "PhysicsDebug用IndexがVertex数の範囲を超えています。");

		// 各頂点に対応するインデックスを取得
		const auto l_vertexZeroIndex = a_indexList[l_baseIndex + k_vertexZeroOffset];
		const auto l_vertexOneIndex  = a_indexList[l_baseIndex + k_vertexOneOffset];
		const auto l_vertexTwoIndex  = a_indexList[l_baseIndex + k_vertexTwoOffset];

		auto& l_triangle = m_triangleList[l_triangleIndex];

		// インデックスから頂点を取得
		l_triangle.mV[k_vertexZeroOffset] = a_vertexList[l_vertexZeroIndex];
		l_triangle.mV[k_vertexOneOffset]  = a_vertexList[l_vertexOneIndex];
		l_triangle.mV[k_vertexTwoOffset]  = a_vertexList[l_vertexTwoIndex];
	}
}