#pragma once

namespace FWK::Graphics
{
	class StaticModelSystem final
	{
	public:

		 StaticModelSystem() = default;
		~StaticModelSystem() = default;

		Struct::StaticModelResult LoadStaticModelForBatchUpload(const Device&			            a_device,
																const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																const std::filesystem::path&        a_filePath, 
																	  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool);


	private:

		AssetStorage<Graphics::StaticModelRecord> m_staticModelStorage = {};

		StaticModelFBXLoader m_loader = {};

		Converter::StaticModelBinaryConverter m_binaryConverter = {};
	};
}