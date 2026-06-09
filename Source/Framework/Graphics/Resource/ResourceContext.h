#pragma once

namespace FWK::Graphics
{
	class DirectCommandQueue;
}

namespace FWK::Graphics
{
	class ResourceContext final
	{
	public:
		
		 ResourceContext() = default;
		~ResourceContext() = default;

		void Deserialize    (const nlohmann::json& a_rootJson);
		bool PostDeserialize(const Device&		   a_device);

		void BeginFrame(const DirectCommandQueue& a_directCommandQueue);

		nlohmann::json Serialize() const;

		const auto& GetREFRTVDescriptorPool() const { return m_rtvDescriptorPool; }
		const auto& GetREFSRVDescriptorPool() const { return m_srvDescriptorPool; }

		const auto& GetREFGPUMemoryAllocator() const { return m_gpuMemoryAllocator; }

		const auto& GetREFUploadSystem () const { return m_uploadSystem; }
		const auto& GetREFTextureSystem() const { return m_textureSystem; }

		const auto& GetREFResourceReleaseContext() const { return m_resourceReleaseContext; }

		auto& GetMutableREFRTVDescriptorPool() { return m_rtvDescriptorPool; }
		auto& GetMutableREFSRVDescriptorPool() { return m_srvDescriptorPool; }

		auto& GetMutableREFUploadSystem () { return m_uploadSystem; }
		auto& GetMutableREFTextureSystem() { return m_textureSystem; }

		auto& GetMutableREFResourceReleaseContext() { return m_resourceReleaseContext; }

	private:

		TypeAlias::RTVDescriptorPool m_rtvDescriptorPool = {};
		TypeAlias::SRVDescriptorPool m_srvDescriptorPool = {};

		GPUMemoryAllocator m_gpuMemoryAllocator = {};
		UploadSystem       m_uploadSystem       = {};


		TextureSystem m_textureSystem = {};

		ResourceReleaseContext m_resourceReleaseContext = {};

		Converter::ResourceContextJsonConverter m_jsonConverter = {};
	};
}