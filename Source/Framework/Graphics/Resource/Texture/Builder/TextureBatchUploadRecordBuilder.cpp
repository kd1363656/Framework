#include "TextureBatchUploadRecordBuilder.h"

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureBatchUploadRecord(const DirectX::ScratchImage&			a_scratchImage, 
																				    const DirectX::TexMetadata&				a_texMetadata, 
																					const Device&							a_device, 
																					const GPUMemoryAllocator&				a_gpuMemoryAllocator, 
																					const std::wstring&						a_filePath, 
																					const TypeAlias::StorageID				a_storageID, 
																						  TypeAlias::SRVDescriptorPool&		a_srvDescriptorPool, 
																						  Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const
{
	auto& l_textureRecord = a_textureBatchUploadRecord.m_textureRecord;


}

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureResource(const DirectX::TexMetadata& a_texMetadata, const GPUMemoryAllocator& a_gpuMemoryAllocator, Graphics::TextureRecord& a_textureRecord) const
{

}