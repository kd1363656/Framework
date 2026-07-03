#pragma once

namespace FWK::Physics
{
	class PhysicsDebugRenderer final : public JPH::DebugRenderer
	{
	public:

		 PhysicsDebugRenderer();
		~PhysicsDebugRenderer() override;

		void ClearFrame();

		void ReserveLineVertexCount();

		void DrawLine(JPH::RVec3Arg a_from, JPH::RVec3Arg a_to, JPH::ColorArg a_color) override;

		void DrawTriangle(JPH::RVec3Arg                   a_vertexZero, 
						  JPH::RVec3Arg                   a_vertexOne, 
						  JPH::RVec3Arg                   a_vertexTwo, 
						  JPH::ColorArg                   a_color,
						  JPH::DebugRenderer::ECastShadow a_castShadow) override;

		JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount) override;

		JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex* a_vertexList, 
													  const int							a_vertexCount,
													  const JPH::uint32*			    a_indexList, 
													  const int						    a_indexCount) override;

		void DrawGeometry(		JPH::RMat44Arg				     a_modelMatrix,
						  const JPH::AABox&					     a_worldSpaceBounds,
						  const float						     a_lodScaleSQ,
								JPH::ColorArg				     a_modelColor,
						  const JPH::DebugRenderer::GeometryRef& a_geometryREF,
							    JPH::DebugRenderer::ECullMode    a_cullMode,
								JPH::DebugRenderer::ECastShadow  a_castShadow,
								JPH::DebugRenderer::EDrawMode    a_drawMode) override;

		void DrawText3D(	  JPH::RVec3Arg	    a_position,
						const JPH::string_view& a_string,
							  JPH::ColorArg		a_color,
					    const float			    a_height) override;

		const auto& GetREFPhysicsDebugRendererQueue() const { return m_debugRendererQueue; }

	private:

		template <typename JoltVectorType>
		TypeAlias::Math::Vector3 ConvertVALVector3(const JoltVectorType& a_vector) const
		{
			return TypeAlias::Math::Vector3
			{
				static_cast<float>(a_vector.GetX()),
				static_cast<float>(a_vector.GetY()),
				static_cast<float>(a_vector.GetZ()),
			};
		}

		TypeAlias::Math::Color ConvertVALColor(JPH::ColorArg a_color) const;

		void AddWorldSpaceAABB(const JPH::AABox& a_worldSpaceBounds, JPH::ColorArg a_color);

		static constexpr float k_colorMAXChannelValue = 255.0F;

		static constexpr std::size_t k_defaultPhysicsDebugLineVertexCapacity = 200000ULL;

		static constexpr std::size_t k_triangleVertexZeroIndex = 0ULL;
		static constexpr std::size_t k_triangleVertexOneIndex  = 1ULL;
		static constexpr std::size_t k_triangleVertexTwoIndex  = 2ULL;

		PhysicsDebugRendererQueue m_debugRendererQueue;
	};
}