#pragma once

namespace FWK::Physics
{
	// カプセルや球などのそのオブジェクトを描画を形成する頂点を一括で作り、
	// 複数フレームにわたって使いまわす為のクラス
	class PhysicsDebugTriangleBatch final : public JPH::RefTargetVirtual
	{
	public:

		// new,delete方式をJoltに合わせるためのマクロ
		JPH_OVERRIDE_NEW_DELETE

		~PhysicsDebugTriangleBatch() override = default;

		static JPH::DebugRenderer::Batch Create(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount);
		
		static JPH::DebugRenderer::Batch Create(const JPH::DebugRenderer::Vertex* a_vertexList, 
												const int						  a_vertexCount, 
												const JPH::uint32*				  a_indexList,
												const int						  a_indexCount);

		void AddRef () override;
		void Release() override;

		const auto& GetREFTriangleList() const { return m_triangleList; }

	private:

		// 外部でこのクラスを生成できないようにする
		PhysicsDebugTriangleBatch() = default;

		void CopyTriangleList(const JPH::DebugRenderer::Triangle* a_triangleList, const int a_triangleCount);

		void CopyIndexedTriangleList(const JPH::DebugRenderer::Vertex* a_vertexList, 
									 const int						   a_vertexCount,
									 const JPH::uint32*				   a_indexList,
									 const int						   a_indexCount);

		static constexpr std::size_t k_vertexZeroOffset = 0ULL;
		static constexpr std::size_t k_vertexOneOffset  = 1ULL;
		static constexpr std::size_t k_vertexTwoOffset  = 2ULL;

		static constexpr std::uint32_t k_singleReferenceCount = 1U;
		static constexpr std::uint32_t k_noReferenceCount     = 0U;

		static constexpr int k_invalidTriangleCount = 0ULL;
		static constexpr int k_invalidIndexCount    = 0ULL;

		std::atomic_uint32_t m_referenceCount = k_noReferenceCount;

		std::vector<JPH::DebugRenderer::Triangle> m_triangleList = {};
	};
}