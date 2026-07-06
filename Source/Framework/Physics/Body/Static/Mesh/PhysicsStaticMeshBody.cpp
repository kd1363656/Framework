#include "PhysicsStaticMeshBody.h"

bool FWK::Physics::PhysicsStaticMeshBody::CreateBody(const Struct::StaticModelData& a_staticModelData, const bool a_isPushBackEnabled, TypeAlias::Math::Matrix& a_worldMatrix)
{
	TypeAlias::Math::Vector3    l_worldScale    = TypeAlias::Math::Vector3::Zero;
	TypeAlias::Math::Quaternion l_worldRotation = TypeAlias::Math::Quaternion::Identity;
	TypeAlias::Math::Vector3    l_worldPosition = TypeAlias::Math::Vector3::Zero;

	// ワールド行列から拡大率、回転、座標を取得
	FWK_ASSERT_RETURN_VALUE_IF(!a_worldMatrix.Decompose(l_worldScale, l_worldRotation, l_worldPosition), "StaticMeshBodyのWorldMatrixを分解できないため、Bodyの作成に失敗しました。", false);

	const bool l_isWorldScaleInvalid = std::abs(l_worldScale.x) <= std::numeric_limits<float>::epsilon() ||
		                               std::abs(l_worldScale.y) <= std::numeric_limits<float>::epsilon() ||
									   std::abs(l_worldScale.z) <= std::numeric_limits<float>::epsilon();

	FWK_ASSERT_RETURN_VALUE_IF(l_isWorldScaleInvalid, "StaticMeshBodyのWorldScaleに0に近い値が含まれているため、Bodyの作成に失敗しました。", false);

	// StaticModelDataから、Scale未適用のMeshShapeを作成する
	const auto& l_meshShape = CreateShape(a_staticModelData);

	FWK_ASSERT_RETURN_VALUE_IF(!l_meshShape, "StaticMeshBody用MeshShapeが無効なため、Bodyの作成に失敗しました。", false);

	const auto& l_physicsWorldScale = Utility::DirectXMathVector3ToJoltVec3(l_worldScale);
	
	// ScaleはMeshShapeの頂点へ直接焼きこまず、
	// 外側のScaledShapeへ設定する
	const JPH::ScaledShapeSettings l_scaledShapeSettings = { l_meshShape.GetPtr(), l_physicsWorldScale };

	const auto& l_scaledShapeResult = l_scaledShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_scaledShapeResult.HasError(), "StaticMeshBody用ScaledShapeの作成に失敗しました。", false);

	const auto& l_scaledShape = l_scaledShapeResult.Get();

	FWK_ASSERT_RETURN_VALUE_IF(!l_scaledShape, "StaticMeshBody用ScaledShapeが無効なため、Bodyの作成に失敗しました。", false);

	// PositionとRotationはBodyへ設定する
	return CreateAndAddStaticBody(l_scaledShape,
								  l_worldPosition,
								  l_worldRotation,
		                          a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticMeshBody::ApplyWorldTransform(TypeAlias::Math::Matrix& a_worldMatrix)
{
	FWK_ASSERT_RETURN_VALUE_IF(GetREFBodyID().IsInvalid(), "StaticMeshBodyが作成されていないため、WorldTransformの反映に失敗しました。", false);

	TypeAlias::Math::Vector3    l_worldScale    = TypeAlias::Math::Vector3::Zero;
	TypeAlias::Math::Quaternion l_worldRotation = TypeAlias::Math::Quaternion::Identity;
	TypeAlias::Math::Vector3    l_worldPosition = TypeAlias::Math::Vector3::Zero;

	// ワールド行列から拡大率、回転、座標を取得
	FWK_ASSERT_RETURN_VALUE_IF(!a_worldMatrix.Decompose(l_worldScale, l_worldRotation, l_worldPosition), "StaticMeshBodyのWorldMatrixを分解できないため、WorldTransformの反映に失敗しました。", false);

	const bool l_isWorldScaleInvalid = std::abs(l_worldScale.x) <= std::numeric_limits<float>::epsilon() ||
		                               std::abs(l_worldScale.y) <= std::numeric_limits<float>::epsilon() ||
									   std::abs(l_worldScale.z) <= std::numeric_limits<float>::epsilon();

	FWK_ASSERT_RETURN_VALUE_IF(l_isWorldScaleInvalid, "StaticMeshBodyのWorldScaleに0に近い値が含まれているため、WorldTransformの反映に失敗しました。", false);

	const auto& l_physicsManager = Physics::PhysicsManager::GetInstance();
	const auto& l_physicsSystem  = l_physicsManager.GetREFPhysicsSystem();
	const auto& l_bodyInterface  = l_physicsSystem.GetBodyInterface    ();

	const auto& l_currentShape = l_bodyInterface.GetShape(GetREFBodyID());

	FWK_ASSERT_RETURN_VALUE_IF(!l_currentShape,                                            "StaticMeshBodyの現在Shapeが無効なため、WorldTransformの反映に失敗しました。",              false);
	FWK_ASSERT_RETURN_VALUE_IF(l_currentShape->GetSubType() != JPH::EShapeSubType::Scaled, "StaticMeshBodyの現在ShapeがScaledShapeではないため、WorldTransformの反映に失敗しました。", false);

	const auto& l_currentScaledShape = static_cast<const JPH::ScaledShape&> (*l_currentShape);
	const auto& l_physicsWorldScale  = Utility::DirectXMathVector3ToJoltVec3(l_worldScale);
	const auto& l_currentWorldScale = l_currentScaledShape.GetScale         ();
	
	// スケールが変更されていれば変更を適用
	if (const bool l_isWorldScaleChanged = std::abs(l_currentWorldScale.GetX() - l_physicsWorldScale.GetX()) > std::numeric_limits<float>::epsilon() ||
		                                   std::abs(l_currentWorldScale.GetY() - l_physicsWorldScale.GetY()) > std::numeric_limits<float>::epsilon() ||
		                                   std::abs(l_currentWorldScale.GetZ() - l_physicsWorldScale.GetZ()) > std::numeric_limits<float>::epsilon(); 
		l_isWorldScaleChanged)
	{
		// 現在のScaledShapeが持つMeshShape本体を再利用する
		const JPH::ScaledShapeSettings& l_scaledShapeSettings = { l_currentScaledShape.GetInnerShape(), l_physicsWorldScale };

		const auto& l_scaledShapeResult = l_scaledShapeSettings.Create();

		FWK_ASSERT_RETURN_VALUE_IF(l_scaledShapeResult.HasError(), "変更後WorldScaleを使用したScaledShapeの作成に失敗しており、WorldTransformの反映に失敗しました。", false);

		const auto& l_scaledShape = l_scaledShapeResult.Get();

		FWK_ASSERT_RETURN_VALUE_IF(!l_scaledShape, "変更後WorldScaleを使用したScaledShapeが無効となっており、WorldTransformの反映に失敗しました。", false);

		if (!ApplyStaticBodyShape(l_scaledShape)) { return false; }
	}

	// PositionとRotationはScaleとは独立してBodyへ反映する
	return ApplyStaticBodyWorldTransform(l_worldRotation, l_worldPosition);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticMeshBody::CreateShape(const Struct::StaticModelData& a_staticModelData) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_staticModelData.m_modelMeshList.empty(), "StaticModelDataのModelMeshリストが空のため、MeshShapeの作成に失敗しました。", {});

	std::size_t l_totalVertexCount = 0ULL;
	std::size_t l_totalIndexCount  = 0ULL;

	// Material単位などで分割されている複数Meshを、
	// 1つのJoltMeshShapeへまとめる
	for (const auto& l_modelMesh : a_staticModelData.m_modelMeshList)
	{
		l_totalVertexCount += l_modelMesh.m_modelVertexList.size();
		l_totalIndexCount  += l_modelMesh.m_indexList.size      ();
	}

	FWK_ASSERT_RETURN_VALUE_IF(l_totalVertexCount == k_invalidTotalVertexCount,              "StaticModelDataの頂点数が0のため、MeshShapeの作成に失敗しました。",                           {});
	FWK_ASSERT_RETURN_VALUE_IF(l_totalIndexCount  == k_invalidTotalIndexCount,               "StaticModelDataのIndex数が0のため、MeshShapeの作成に失敗しました。",                          {});
	FWK_ASSERT_RETURN_VALUE_IF(l_totalVertexCount > std::numeric_limits<JPH::uint32>::max(), "StaticModelDataの頂点数が32bit Indexの上限を超えているため、MeshShapeの作成に失敗しました。", {});

	JPH::VertexList          l_triangleVertexList  = {};
	JPH::IndexedTriangleList l_indexedTriangleList = {};

	l_triangleVertexList.reserve (l_totalVertexCount);
	l_indexedTriangleList.reserve(l_totalIndexCount / Constant::k_triangleVertexCount);

	for (const auto& l_modelMesh : a_staticModelData.m_modelMeshList)
	{
		const auto& l_modelVertexList = l_modelMesh.m_modelVertexList;
		const auto& l_modelIndexList  = l_modelMesh.m_indexList;

		FWK_ASSERT_RETURN_VALUE_IF(l_modelVertexList.empty(),                                                              "StaticModelMeshの頂点リストが空のため、MeshShapeの作成に失敗しました。",         {});
		FWK_ASSERT_RETURN_VALUE_IF(l_modelIndexList.empty(),                                                               "StaticModelMeshのIndexリストが空のため、MeshShapeの作成に失敗しました。",        {});
		FWK_ASSERT_RETURN_VALUE_IF((l_modelIndexList.size() % Constant::k_triangleVertexCount) != Constant::k_noRemainder, "StaticModelMeshのIndex数が3の倍数ではないため、MeshShapeの作成に失敗しました。", {});

		const auto l_vertexOffset = static_cast<JPH::uint32>(l_triangleVertexList.size());

		for (const auto& l_modelVertex : l_modelVertexList)
		{
			const auto& l_position = l_modelVertex.m_position;

			// StaticModelDataにはFBXNodeTransformが既に焼きこまれている、
			// ここでは外部WorldMatrixを適用しない
			l_triangleVertexList.emplace_back(l_position.x, l_position.y, l_position.z);
		}

		for (std::size_t l_indexOffset = 0ULL; l_indexOffset < l_modelIndexList.size(); l_indexOffset += Constant::k_triangleVertexCount)
		{
			const auto l_localIndexZero = l_modelIndexList[l_indexOffset + k_triangleIndexZeroOffset];
			      auto l_localIndexOne  = l_modelIndexList[l_indexOffset + k_triangleIndexOneOffset];
			      auto l_localIndexTwo  = l_modelIndexList[l_indexOffset + k_triangleIndexTwoOffset];

			const bool l_isIndexInvalid = l_localIndexZero >= l_modelVertexList.size() ||
				                          l_localIndexOne  >= l_modelVertexList.size() ||
				                          l_localIndexTwo  >= l_modelVertexList.size();

			FWK_ASSERT_RETURN_VALUE_IF(l_isIndexInvalid, "StaticModelMeshに頂点数を超えるIndexが存在するため、MeshShapeの作成に失敗しました。", {});

			const auto& l_vertexZero = l_modelVertexList[l_localIndexZero];
			const auto& l_vertexOne  = l_modelVertexList[l_localIndexOne];
			const auto& l_vertexTwo  = l_modelVertexList[l_localIndexTwo];

			// JoltのMeshShapeは反時計回りを表面として扱う
			// モデル法線と幾何法線が逆ならIndex順を反転する
			const auto& l_faceNormal    = (l_vertexOne.m_position - l_vertexZero.m_position).Cross(l_vertexTwo.m_position - l_vertexZero.m_position);
			const auto& l_averageNormal = (l_vertexZero.m_normal  + l_vertexOne.m_normal + l_vertexTwo.m_normal) / Constant::k_triangleVertexCount;

			// 現在のIndex順から求めた面法線と、
			// モデル頂点が持つ法線の方向が逆の場合は、
			// Index順を反転して三角形の表裏を合わせる
			if (l_faceNormal.LengthSquared()      > std::numeric_limits<float>::epsilon() &&
				l_averageNormal.LengthSquared()   > std::numeric_limits<float>::epsilon() &&
				l_faceNormal.Dot(l_averageNormal) < k_oppositeNormalDotThreshold)
			{
				std::swap(l_localIndexOne, l_localIndexTwo);
			}

			l_indexedTriangleList.emplace_back(l_vertexOffset + l_localIndexZero, 
				                               l_vertexOffset + l_localIndexOne,
				                               l_vertexOffset + l_localIndexTwo, 
				                               k_defaultMaterialIndex);
		}
	}

	FWK_ASSERT_RETURN_VALUE_IF(l_triangleVertexList.empty(),  "MeshShapeへ渡す頂点が存在しないため、MeshShapeの作成に失敗しました。",   {});
	FWK_ASSERT_RETURN_VALUE_IF(l_indexedTriangleList.empty(), "MeshShapeへ渡す三角形が存在しないため、MeshShapeの作成に失敗しました。", {});

	const JPH::MeshShapeSettings l_meshShapeSettings = { std::move(l_triangleVertexList), std::move(l_indexedTriangleList) };

	const auto l_shapeResult = l_meshShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticModelDataからMeshShapeを作成できておらず、MeshShapeの作成に失敗しました。", {});

	return l_shapeResult.Get();
}