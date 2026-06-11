#pragma once

namespace FWK::Graphics
{
	class FrameResource final
	{
	private:

		using ConstantBufferUploaderMap = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<ConstantBufferUploaderBase>>;

	public:

		 FrameResource() = default;
		~FrameResource() = default;

		void INIT       ();
		bool Create     (const Device&		   a_device);
		void Deserialize(const nlohmann::json& a_rootJson);

		void BeginFrame();

		nlohmann::json Serialize() const;

		void AddConstantBufferUploader(const std::shared_ptr<ConstantBufferUploaderBase>& a_constantBufferUploader);

		const auto& GetREFConstantBufferUploaderList() const { return m_constantBufferUploaderList; }

		const auto& GetREFDirectCommandAllocator() const { return m_directCommandAllocator; }

	private:

		void RemoveExpiredConstantBufferUploaderList();
		void RemoveExpiredConstantBufferUploaderMap ();

		ConstantBufferUploaderMap m_constantBufferUploaderMap = {};

		std::vector<std::shared_ptr<ConstantBufferUploaderBase>> m_constantBufferUploaderList = {};

		std::shared_ptr<DirectCommandAllocator> m_directCommandAllocator = nullptr;

		Converter::FrameResourceJsonConverter m_jsonConverter = {};
	};
}