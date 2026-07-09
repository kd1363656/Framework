#pragma once

namespace FWK::Graphics
{
	class FrameResource final
	{
	private:

		using DynamicBufferUploaderMap = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<DynamicBufferUploaderBase>>;

	public:

		 FrameResource() = default;
		~FrameResource() = default;

		void INIT();

		bool Create(const Device&			  a_device,
				    const GPUMemoryAllocator& a_gpuMemoryAllocator,
					const Window::ClientSize& a_clientSize,
						  ResourceContext&	  a_resourceContext);

		bool Resize(const Device&			  a_device,
					const GPUMemoryAllocator& a_gpuMemoryAllocator,
					const Window::ClientSize& a_clientSize,
					const UINT64&			  a_retiredFenceValue,
						  ResourceContext&    a_resourceContext) const;

		void Deserialize(const nlohmann::json& a_rootJson);

		void BeginFrame();

		nlohmann::json Serialize() const;

		void AddDynamicBufferUploader(const std::shared_ptr<DynamicBufferUploaderBase>& a_dynamicBufferUploader);

		template <Concept::IsDerivedDynamicBufferUploaderBaseConcept Type>
		std::weak_ptr<Type> FindPTRDynamicBufferUploader() const
		{
			const auto& l_itr = m_dynamicBufferUploaderMap.find(Type::GetREFTypeINFO().k_staticTypeID);

			if (l_itr == m_dynamicBufferUploaderMap.end()) { return std::weak_ptr<Type>(); }

			const auto l_dynamicBufferUploader = l_itr->second.lock();

			if (!l_dynamicBufferUploader) { return std::weak_ptr<Type>(); }

			return std::static_pointer_cast<Type>(l_dynamicBufferUploader);
		}

		const auto& GetREFConstantBufferUploaderList() const { return m_dynamicBufferUploaderList; }

		const auto& GetREFDirectCommandAllocator () const { return m_directCommandAllocator; }
		const auto& GetREFComputeCommandAllocator() const { return m_computeCommandAllocator; }
		
		const auto& GetREFRenderGraphFrameResource  () const { return m_renderGraphFrameResource; }

		auto& GetMutableREFRenderGraphFrameResource() { return m_renderGraphFrameResource; }

	private:

		void RemoveExpiredConstantBufferUploaderList();
		void RemoveExpiredConstantBufferUploaderMap ();

		DynamicBufferUploaderMap m_dynamicBufferUploaderMap = {};

		std::vector<std::shared_ptr<DynamicBufferUploaderBase>> m_dynamicBufferUploaderList = {};

		std::shared_ptr<TypeAlias::DirectCommandAllocator>  m_directCommandAllocator  = nullptr;
		std::shared_ptr<TypeAlias::ComputeCommandAllocator> m_computeCommandAllocator = nullptr;

		RenderGraphFrameResource m_renderGraphFrameResource = {};

		Converter::FrameResourceJsonConverter m_jsonConverter = {};
	};
}