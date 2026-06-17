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

		void ProcessPendingTextureUploads     ();
		void ReleaseCompletedDeferredResources(const DirectCommandQueue& a_directCommandQueue);

		nlohmann::json Serialize() const;

		const auto& GetREFShaderCompiler() const { return m_shaderCompiler; }

		const auto& GetREFRTVDescriptorPool() const { return m_rtvDescriptorPool; }
		const auto& GetREFSRVDescriptorPool() const { return m_srvDescriptorPool; }
		const auto& GetREFDSVDescriptorPool() const { return m_dsvDescriptorPool; }

		const auto& GetREFGPUMemoryAllocator() const { return m_gpuMemoryAllocator; }

		const auto& GetREFUploadSystem () const { return m_uploadSystem; }
		const auto& GetREFTextureSystem() const { return m_textureSystem; }

		const auto& GetREFResourceReleaseContext() const { return m_resourceReleaseContext; }

		auto& GetMutableREFRTVDescriptorPool() { return m_rtvDescriptorPool; }
		auto& GetMutableREFSRVDescriptorPool() { return m_srvDescriptorPool; }
		auto& GetMutableREFDSVDescriptorPool() { return m_dsvDescriptorPool; }

		auto& GetMutableREFUploadSystem     () { return m_uploadSystem; }
		auto& GetMutableREFTextureSystem    () { return m_textureSystem; }
		auto& GetMutableREFStaticModelSystem() { return m_staticModelSystem; }

		auto& GetMutableREFResourceReleaseContext() { return m_resourceReleaseContext; }

	private:

		ShaderCompiler m_shaderCompiler = {};

		TypeAlias::RTVDescriptorPool m_rtvDescriptorPool = {};
		TypeAlias::SRVDescriptorPool m_srvDescriptorPool = {};
		TypeAlias::DSVDescriptorPool m_dsvDescriptorPool = {};

		GPUMemoryAllocator m_gpuMemoryAllocator = {};
		UploadSystem       m_uploadSystem       = {};

		TextureSystem     m_textureSystem     = {};
		StaticModelSystem m_staticModelSystem = {};

		ResourceReleaseContext m_resourceReleaseContext = {};

		Converter::ResourceContextJsonConverter m_jsonConverter = {};
	};
}