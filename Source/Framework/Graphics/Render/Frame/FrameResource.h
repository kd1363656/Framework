#pragma once

namespace FWK::Graphics
{
	class FrameResource final
	{
	public:

		 FrameResource() = default;
		~FrameResource() = default;

		void INIT       ();
		bool Create     (const Device&		   a_device);
		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		const auto& GetREFDirectCommandAllocator() const { return m_directCommandAllocator; }

	private:

		std::shared_ptr<DirectCommandAllocator> m_directCommandAllocator = nullptr;
	};
}