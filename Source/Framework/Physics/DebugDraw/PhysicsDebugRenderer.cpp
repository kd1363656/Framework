#include "PhysicsDebugRenderer.h"

FWK::Physics::PhysicsDebugRenderer::PhysicsDebugRenderer() : 
	m_debugDrawQueue()
{
	// JoltのDebugRendererは、派生クラスのコンストラクタでInitialize()を呼ぶ必要がある。
	// これによりJoltの内部のDebugRenderer::sInstanceなどが準備される
	Initialize();
}
FWK::Physics::PhysicsDebugRenderer::~PhysicsDebugRenderer()
{
	// Jolt側のDebugRenderer::sInstanceは生ポインタのため
	// 破棄時に自身を示している場合だけnullptrへ戻す
	if (JPH::DebugRenderer::sInstance != this) { return; }

	JPH::DebugRenderer::sInstance = nullptr;
}

void FWK::Physics::PhysicsDebugRenderer::ClearFrame()
{
	m_debugDrawQueue.ClearFrame();
}

void FWK::Physics::PhysicsDebugRenderer::ReserveLineVertexCount()
{
	m_debugDrawQueue.ReserveLineVertexCount(k_defaultPhysicsDebugLineVertexCapacity);
}

void FWK::Physics::PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg a_from, JPH::RVec3Arg a_to, JPH::ColorArg a_color)
{
	m_debugDrawQueue.AddLine(ConvertVALVector3(a_from), ConvertVALVector3(a_to), ConvertVALColor(a_color));
}
void FWK::Physics::PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg                   a_vertexZero, 
													  JPH::RVec3Arg                   a_vertexOne,
													  JPH::RVec3Arg                   a_vertexTwo,
													  JPH::ColorArg                   a_color,
													  JPH::DebugRenderer::ECastShadow a_castShadow)
{
	(void)a_castShadow;

	// 3辺のLineListとして描画する
	m_debugDrawQueue.AddTriangleWire(ConvertVALVector3(a_vertexZero), 
									 ConvertVALVector3(a_vertexOne),
									 ConvertVALVector3(a_vertexTwo), 
									 ConvertVALColor(a_color));
}

JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount)
{
	return PhysicsDebugTriangleBatch::Create(a_triangleList, a_triangleCount);
}
JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* a_vertexList, 
																				  const int					        a_vertexCount, 
																				  const JPH::uint32*				a_indexList,
																				  const int						    a_indexCount)
{
	return PhysicsDebugTriangleBatch::Create(a_vertexList,
											 a_vertexCount,
											 a_indexList,
											 a_indexCount);
}

void FWK::Physics::PhysicsDebugRenderer::DrawGeometry(		JPH::RMat44Arg                   a_modelMatrix, 
													  const JPH::AABox&	                     a_worldSpaceBounds, 
													  const float		                     a_lodScaleSQ, 
														    JPH::ColorArg                    a_modelColor,
													  const JPH::DebugRenderer::GeometryRef& a_geometryREF, 
														    JPH::DebugRenderer::ECullMode    a_cullMode, 
														    JPH::DebugRenderer::ECastShadow  a_castShadow, 
															JPH::DebugRenderer::EDrawMode    a_drawMode)
{
	(void)a_worldSpaceBounds;
	(void)a_lodScaleSQ;
	(void)a_cullMode;
	(void)a_castShadow;
	(void)a_drawMode;

	FWK_ASSERT_RETURN_IF(!a_geometryREF.GetPtr(),      "PhysicsDebug用Geometryが無効なため、DrawGeometryに失敗しました。");
	FWK_ASSERT_RETURN_IF(a_geometryREF->mLODs.empty(), "PhysicsDebug用GeometryにLODが存在しません。");

	// 最も表際なLODを使用する。
	const auto& l_lod = a_geometryREF->mLODs.back();

	FWK_ASSERT_RETURN_IF(!l_lod.mTriangleBatch.GetPtr(), "PhysicsDebug用TriangleBacthが無効です。");

	const auto& l_triangleBatch = static_cast<const PhysicsDebugTriangleBatch&>(*l_lod.mTriangleBatch.GetPtr());

	for (const auto& l_triangle : l_triangleBatch.GetREFTriangleList())
	{
		// Bach内の頂点はShapwのローカル座標なので、
		// Joltから渡されたModelMatrixでワールド座標へ変換する
		const JPH::RVec3 l_vertexZero = a_modelMatrix * JPH::Vec3(l_triangle.mV[k_triangleVertexZeroIndex].mPosition);
		const JPH::RVec3 l_vertexOne  = a_modelMatrix * JPH::Vec3(l_triangle.mV[k_triangleVertexOneIndex].mPosition);
		const JPH::RVec3 l_vertexTwo  = a_modelMatrix * JPH::Vec3(l_triangle.mV[k_triangleVertexTwoIndex].mPosition);

		// Joltのモデル色とBach側の頂点色を乗算する
		const JPH::Color l_triangleColor = a_modelColor * l_triangle.mV[k_triangleVertexZeroIndex].mColor;

		m_debugDrawQueue.AddTriangleWire(ConvertVALVector3(l_vertexZero),
										 ConvertVALVector3(l_vertexOne),
										 ConvertVALVector3(l_vertexTwo),
										 ConvertVALColor(l_triangleColor));
	}
}

void FWK::Physics::PhysicsDebugRenderer::DrawText3D(	  JPH::RVec3Arg     a_position, 
												    const JPH::string_view& a_string, 
														  JPH::ColorArg	    a_color, 
												    const float			    a_height)
{
	(void)a_position;
	(void)a_string;
	(void)a_color;
	(void)a_height;
}

FWK::TypeAlias::Math::Color FWK::Physics::PhysicsDebugRenderer::ConvertVALColor(JPH::ColorArg a_color) const
{
	return TypeAlias::Math::Color
	{
		(a_color.r) / k_colorMAXChannelValue,
		(a_color.g) / k_colorMAXChannelValue,
		(a_color.b) / k_colorMAXChannelValue,
		(a_color.a) / k_colorMAXChannelValue,
	};
}

void FWK::Physics::PhysicsDebugRenderer::AddWorldSpaceAABB(const JPH::AABox& a_worldSpaceBounds, JPH::ColorArg a_color)
{
	m_debugDrawQueue.AddAABB(ConvertVALVector3(a_worldSpaceBounds.mMin), ConvertVALVector3(a_worldSpaceBounds.mMax), ConvertVALColor(a_color));
}