#include "PhysicsDebugRenderer.h"

FWK::Graphics::JoltPhysicDebugRenderer::JoltPhysicDebugRenderer()
{
	Initialize();
}
FWK::Graphics::JoltPhysicDebugRenderer::~JoltPhysicDebugRenderer()
{

}

void FWK::Graphics::JoltPhysicDebugRenderer::DrawLine(JPH::RVec3Arg a_from, JPH::RVec3Arg a_to, JPH::ColorArg a_inColor)
{

}
void FWK::Graphics::JoltPhysicDebugRenderer::DrawTriangle(JPH::RVec3Arg                   a_vertexZero, 
														  JPH::RVec3Arg                   a_vertexOne,
														  JPH::RVec3Arg                   a_vertexTwo,
														  JPH::ColorArg                   a_color,
														  JPH::DebugRenderer::ECastShadow a_castShadow)
{

}

JPH::DebugRenderer::Batch FWK::Graphics::JoltPhysicDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle * a_triangle, const int a_triangleCount)
{
	return JPH::DebugRenderer::Batch();
}

JPH::DebugRenderer::Batch FWK::Graphics::JoltPhysicDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* a_vertexList, 
																					  const int							a_vertexCount, 
																					  const JPH::uint32*				a_indexList,
																					  const int						    a_indexCount)
{
	return JPH::DebugRenderer::Batch();
}

void FWK::Graphics::JoltPhysicDebugRenderer::DrawGeometry(		JPH::RMat44Arg                   a_modelMatrix, 
														  const JPH::AABox&	                     a_worldSpaceBounds, 
														  const float		                     a_loadScaleSQ, 
																JPH::ColorArg                    a_modelColor,
														  const JPH::DebugRenderer::GeometryRef& a_gometryREF, 
																JPH::DebugRenderer::ECullMode    a_cullMode, JPH::DebugRenderer::ECastShadow a_castShadow, JPH::DebugRenderer::EDrawMode a_drawMode)
{

}

void FWK::Graphics::JoltPhysicDebugRenderer::DrawText3D(	  JPH::RVec3Arg     a_position, 
														const JPH::string_view& a_string, 
															  JPH::ColorArg		a_color, 
														const float				a_height)
{

}

FWK::TypeAlias::Math::Color FWK::Graphics::JoltPhysicDebugRenderer::ConvertVALColor(JPH::ColorArg& a_color) const
{
	return TypeAlias::Math::Color();
}

void FWK::Graphics::JoltPhysicDebugRenderer::AddWorldSpaceAABB(const JPH::AABox& a_worldSpaceBounds, JPH::ColorArg a_color)
{

}