#include "PhysicsDebugRenderer.h"

FWK::Physics::PhysicsDebugRenderer::PhysicsDebugRenderer() : 
	m_debugDrawQueue()
{
	// JoltのDebugRendererは、派生クラスのコンストラクタでInitialize(9を呼ぶ必要がある。
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
													  JPH::DebugRenderer::ECastShadow)
{
	// 3辺のLineListとして描画する
	m_debugDrawQueue.AddTriangleWire(ConvertVALVector3(a_vertexZero), 
									 ConvertVALVector3(a_vertexOne),
									 ConvertVALVector3(a_vertexTwo), 
									 ConvertVALColor(a_color));
}

JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* a_triangle, const int a_triangleCount)
{
	return JPH::DebugRenderer::Batch();
}

JPH::DebugRenderer::Batch FWK::Physics::PhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* a_vertexList, 
																				  const int					        a_vertexCount, 
																				  const JPH::uint32*				a_indexList,
																				  const int						    a_indexCount)
{
	return JPH::DebugRenderer::Batch();
}

void FWK::Physics::PhysicsDebugRenderer::DrawGeometry(		JPH::RMat44Arg                   a_modelMatrix, 
													  const JPH::AABox&	                     a_worldSpaceBounds, 
													  const float		                     a_loadScaleSQ, 
														    JPH::ColorArg                    a_modelColor,
													  const JPH::DebugRenderer::GeometryRef& a_gometryREF, 
														    JPH::DebugRenderer::ECullMode    a_cullMode, 
														    JPH::DebugRenderer::ECastShadow  a_castShadow, 
															JPH::DebugRenderer::EDrawMode    a_drawMode)
{
	AddWorldSpaceAABB(a_worldSpaceBounds, a_modelColor);
}

void FWK::Physics::PhysicsDebugRenderer::DrawText3D(	  JPH::RVec3Arg     a_position, 
												    const JPH::string_view& a_string, 
														  JPH::ColorArg	    a_color, 
												    const float			    a_height)
{

}

FWK::TypeAlias::Math::Color FWK::Physics::PhysicsDebugRenderer::ConvertVALColor(JPH::ColorArg& a_color) const
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