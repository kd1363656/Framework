#include "ConstantBufferUploaderBase.h"

FWK::Graphics::ConstantBufferUploaderBase::ConstantBufferUploaderBase(const UINT64& a_constantBufferTypeSize) : 
	m_uploadBuffer(),

	m_constantBufferTypeSize(a_constantBufferTypeSize),

	m_createConstantBufferCount(k_invalidCreateConstantBufferCount),

	m_currentBufferIndex(k_initialBufferIndex)
{}
FWK::Graphics::ConstantBufferUploaderBase::~ConstantBufferUploaderBase() = default;

void FWK::Graphics::ConstantBufferUploaderBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ConstantBufferUploaderBase::Create(const Device& a_device)
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_createConstantBufferCount == k_invalidCreateConstantBufferCount, "定数バッファの作成個数が0のため、作成処理に失敗しました。", false);

	// 送る定数バッファの型サイズを256バイトにアライメントする
	const auto& l_alignedTypeSize    = Utility::AlignUp(sizeof(m_constantBufferTypeSize), k_constantBufferAlignment);
	const auto& l_constantBufferSize = m_createConstantBufferCount * l_alignedTypeSize;
	
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_uploadBuffer.Create(a_device, l_constantBufferSize), "定数バッファの生成処理に失敗しました。", false);

	return true;
}

void FWK::Graphics::ConstantBufferUploaderBase::BeginFrame()
{
	m_currentBufferIndex = k_initialBufferIndex;
}

nlohmann::json FWK::Graphics::ConstantBufferUploaderBase::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}