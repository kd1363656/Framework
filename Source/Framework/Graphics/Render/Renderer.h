#pragma once

namespace FWK::Graphics
{
	class Renderer final
	{
	public:

		 Renderer() = default;
		~Renderer() = default;

		void Deserialize    (const nlohmann::json& a_rootJson);
		void PostDeserialize(const Device&		   a_device);
		
		void BeginFrame();
		void EndFrame  ();

		nlohmann::json Serialize() const;

		void AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource);

		const auto& GetREFFrameResourceList() const { return m_frameResourceList; }

	private:

		static constexpr std::size_t k_initialFrameResourceIndex   = 0ULL;
		static constexpr std::size_t k_frameResourceIndexIncrement = 1ULL;

		std::vector<std::shared_ptr<FrameResource>> m_frameResourceList = {};

		std::weak_ptr<FrameResource> m_currentFrameResource = {};

		DirectCommandQueue m_directCommandQueue = {};
		DirectCommandList  m_directCommandList  = {};

		Converter::RendererJsonConverter m_jsonConverter = {};

		std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;
	};
}