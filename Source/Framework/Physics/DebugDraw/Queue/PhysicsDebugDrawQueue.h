#pragma once

namespace FWK::Physics
{
	class PhysicsDebugDrawQueue final
	{
	public:

		 PhysicsDebugDrawQueue() = default;
		~PhysicsDebugDrawQueue() = default;

		void ClearFrame();

		void AddLine(const TypeAlias::Math::Vector3& a_startPosition, const TypeAlias::Math::Vector3& a_endPosition, const TypeAlias::Math::Color& a_color);

		void AddTriangleWire(const TypeAlias::Math::Vector3& a_vertexZero,
							 const TypeAlias::Math::Vector3& a_vertexOne, 
							 const TypeAlias::Math::Vector3& a_vertexTwo, 
							 const TypeAlias::Math::Color&   a_color);

		void AddAABB(const TypeAlias::Math::Vector3& a_minPosition, const TypeAlias::Math::Vector3& a_maxPosition, const TypeAlias::Math::Color& a_color);

		const auto& GetREFLineVertexList() const { return m_lineVertexList; }

		bool HasLineVertex() const { return !m_lineVertexList.empty(); }

	private:

		struct AABBCornerSelector
		{
			bool m_useMAXX = false;
			bool m_useMAXY = false;
			bool m_useMAXZ = false;
		};

		struct AABBEdgeIndex 
		{
			std::size_t m_startCornerIndex = k_initialCornerIndex;
			std::size_t m_endCornerIndex   = k_initialCornerIndex;
		};

		void AddLineVertex(const TypeAlias::Math::Vector3& a_position, const TypeAlias::Math::Color& a_color);

		TypeAlias::Math::Vector3 CreateAABBCorner(const TypeAlias::Math::Vector3& a_minPosition, const TypeAlias::Math::Vector3& a_maxPosition, const AABBCornerSelector a_cornerSelector) const;

		static constexpr std::size_t k_initialCornerIndex = 0ULL;
		static constexpr std::size_t k_aabbCornerCount    = 8ULL;
		static constexpr std::size_t k_aabbEdgeCount      = 12ULL;

		static constexpr bool k_useMINSide = false;
		static constexpr bool k_useMAXSide = true;

		static constexpr std::array<AABBCornerSelector, k_aabbCornerCount> k_aabbCornerSelectorList =
		{
			AABBCornerSelector{ k_useMINSide, k_useMINSide, k_useMINSide },
			AABBCornerSelector{ k_useMAXSide, k_useMINSide, k_useMINSide },
			AABBCornerSelector{ k_useMAXSide, k_useMAXSide, k_useMINSide },
			AABBCornerSelector{ k_useMINSide, k_useMAXSide, k_useMINSide },
			AABBCornerSelector{ k_useMINSide, k_useMINSide, k_useMAXSide },
			AABBCornerSelector{ k_useMAXSide, k_useMINSide, k_useMAXSide },
			AABBCornerSelector{ k_useMAXSide, k_useMAXSide, k_useMAXSide },
			AABBCornerSelector{ k_useMINSide, k_useMAXSide, k_useMAXSide },
		};

		static constexpr std::array<AABBEdgeIndex, k_aabbEdgeCount> k_aabbEdgeIndexList =
		{
			AABBEdgeIndex{ 0ULL, 1ULL },
			AABBEdgeIndex{ 1ULL, 2ULL },
			AABBEdgeIndex{ 2ULL, 3ULL },
			AABBEdgeIndex{ 3ULL, 0ULL },
			AABBEdgeIndex{ 4ULL, 5ULL },
			AABBEdgeIndex{ 5ULL, 6ULL },
			AABBEdgeIndex{ 6ULL, 7ULL },
			AABBEdgeIndex{ 7ULL, 4ULL },
			AABBEdgeIndex{ 0ULL, 4ULL },
			AABBEdgeIndex{ 1ULL, 5ULL },
			AABBEdgeIndex{ 2ULL, 6ULL },
			AABBEdgeIndex{ 3ULL, 7ULL },
		};

		std::vector<Struct::VBPhysicsDebug> m_lineVertexList = {};
	};
}