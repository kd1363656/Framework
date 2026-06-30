#include "DynamicBufferUploaderBase.h"

FWK::Graphics::DynamicBufferUploaderBase::DynamicBufferUploaderBase(const UINT64& a_bufferTypeSize) : 
	k_typeSize(a_bufferTypeSize),

	m_uploadBuffer(),

	m_createCount(k_invalidCreateCount),

	m_currentElementIndex(k_initialElementBufferIndex)
{}
FWK::Graphics::DynamicBufferUploaderBase::~DynamicBufferUploaderBase() = default;

void FWK::Graphics::DynamicBufferUploaderBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::DynamicBufferUploaderBase::BeginFrame()
{
	m_currentElementIndex = k_initialElementBufferIndex;
}

nlohmann::json FWK::Graphics::DynamicBufferUploaderBase::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::DynamicBufferUploaderBase::CreateUploadBuffer(const Device& a_device, const UINT64& a_alignment)
{
	FWK_ASSERT_RETURN_VALUE_IF(m_createCount == k_invalidCreateCount, "バッファの作成個数が0のため、作成処理に失敗しました。", false);
	
	// 送る定数バッファの型サイズを256バイトにアライメントする
	const auto& l_alignedTypeSize = Utility::AlignUp(k_typeSize, a_alignment);
	const auto& l_totalSize       = m_createCount * l_alignedTypeSize;

	FWK_ASSERT_RETURN_VALUE_IF(l_totalSize == Constant::k_invalidBufferSize,  "作成バッファーサイズが0のためバッファの生成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_uploadBuffer.Create(a_device, l_totalSize), "バッファの生成処理に失敗しました。", false);

	return true;
}

UINT64 FWK::Graphics::DynamicBufferUploaderBase::AllocateElementRange(const UINT64& a_elementCount)
{
	const auto l_startElementIndex = m_currentElementIndex;

	m_currentElementIndex += a_elementCount;

	FWK_ASSERT_RETURN_VALUE_IF(m_createCount < m_currentElementIndex, "要素範囲を超えており、Allocate処理に失敗しました。", k_invalidElementBufferIndex);

	return l_startElementIndex;
}