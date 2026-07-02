#include "PhysicsDebugDrawQueue.h"

void FWK::Physics::PhysicsDebugDrawQueue::ClearFrame()
{
	m_lineVertexList.clear();
}

void FWK::Physics::PhysicsDebugDrawQueue::AddLine(const TypeAlias::Math::Vector3& a_startPosition, const TypeAlias::Math::Vector3& a_endPosition, const TypeAlias::Math::Color& a_color)
{
	AddLineVertex(a_startPosition, a_color);
	AddLineVertex(a_endPosition,   a_color);
}

void FWK::Physics::PhysicsDebugDrawQueue::AddTriangleWire(const TypeAlias::Math::Vector3& a_vertexZero,
														  const TypeAlias::Math::Vector3& a_vertexOne,
														  const TypeAlias::Math::Vector3& a_vertexTwo,
														  const TypeAlias::Math::Color&   a_color)
{
	AddLine(a_vertexZero, a_vertexOne,  a_color);
	AddLine(a_vertexOne,  a_vertexTwo,  a_color);
	AddLine(a_vertexTwo,  a_vertexZero, a_color);
}

void FWK::Physics::PhysicsDebugDrawQueue::AddAABB(const TypeAlias::Math::Vector3& a_minPosition, const TypeAlias::Math::Vector3& a_maxPosition, const TypeAlias::Math::Color& a_color)
{
	std::array<TypeAlias::Math::Vector3, k_aabbCornerCount> l_cornerList = {};

	for (std::size_t l_cornderIndex = 0ULL; l_cornderIndex < k_aabbCornerCount; ++l_cornderIndex)
	{
		l_cornerList[l_cornderIndex] = CreateAABBCorner(a_minPosition, a_maxPosition, k_aabbCornerSelectorList[l_cornderIndex]);
	}

	for (const auto& l_edgeIndex : k_aabbEdgeIndexList)
	{
		AddLine(l_cornerList[l_edgeIndex.m_startCornerIndex], l_cornerList[l_edgeIndex.m_endCornerIndex], a_color);
	}
}

void FWK::Physics::PhysicsDebugDrawQueue::AddLineVertex(const TypeAlias::Math::Vector3& a_position, const TypeAlias::Math::Color& a_color)
{
	Struct::VBPhysicsDebug l_vbPhysicsDebug = { a_position, a_color };

	m_lineVertexList.emplace_back(std::move(l_vbPhysicsDebug));
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsDebugDrawQueue::CreateAABBCorner(const TypeAlias::Math::Vector3& a_minPosition, const TypeAlias::Math::Vector3& a_maxPosition, const AABBCornerSelector a_cornerSelector) const
{
	return TypeAlias::Math::Vector3{ a_cornerSelector.m_useMAXX ? a_maxPosition.x : a_minPosition.x,
									 a_cornerSelector.m_useMAXY ? a_maxPosition.y : a_minPosition.y, 
									 a_cornerSelector.m_useMAXZ ? a_maxPosition.z : a_minPosition.z, };
}