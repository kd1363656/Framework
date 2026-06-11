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

		template <Concept::IsDerivedConstantBufferBaseConcept ConstantBufferType>
		std::weak_ptr<ConstantBufferType> FindPTRConstantBufferUploader() const
		{
			const auto& l_itr = m_constantBufferUploaderMap.find(ConstantBufferType::GetREFTypeINFO().k_staticTypeID);

			if (l_itr == m_constantBufferUploaderMap.end()) { return std::weak_ptr<ConstantBufferType>(); }

			const auto l_constantBufferUploader = l_itr->second.lock();

			if (!l_constantBufferUploader) { return std::weak_ptr<ConstantBufferType>(); }

			return std::static_pointer_cast<ConstantBufferType>(l_constantBufferUploader);
		}

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