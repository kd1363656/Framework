#include "DynamicBufferUploaderBase.h"

FWK::Graphics::DynamicBufferUploaderBase::DynamicBufferUploaderBase(const UINT64& a_typeSize, const bool a_shouldAdvanceWritePosition) :
	k_typeSize(a_typeSize),

	k_shouldAdvanceWritePosition(a_shouldAdvanceWritePosition),

	m_uploadBuffer(),

	m_jsonConverter(),

	m_createCount(k_invalidCreateCount),

	m_currentElementIndex(k_initialElementBufferIndex),
	m_elementStrideSize  (k_initialElementStrideSize)
{}
FWK::Graphics::DynamicBufferUploaderBase::~DynamicBufferUploaderBase() = default;

FWK::Graphics::DynamicBufferUploaderBase::DynamicBufferUploaderBase(DynamicBufferUploaderBase&& a_other) noexcept :
	k_typeSize(a_other.k_typeSize),

	k_shouldAdvanceWritePosition(a_other.k_shouldAdvanceWritePosition),

	m_uploadBuffer(std::move(a_other.m_uploadBuffer)),

	m_jsonConverter(std::move(a_other.m_jsonConverter)),

	m_createCount(a_other.m_createCount),

	m_currentElementIndex(a_other.m_currentElementIndex),
	m_elementStrideSize  (a_other.m_elementStrideSize)
{
	// UploadBufferの所有権は移動先へ渡っている。
	// 移動元が作成済み状態に見えないよう、
	// 可変状態を初期値へ戻す。
	a_other.m_createCount = k_invalidCreateCount;

	a_other.m_currentElementIndex = k_initialElementBufferIndex;

	a_other.m_elementStrideSize = k_initialElementBufferIndex;
}

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
	
	// 送るバッファの型サイズを所定のバイト数にアライメントする
	const auto& l_alignedTypeSize = Utility::AlignUp(k_typeSize, a_alignment);

	FWK_ASSERT_RETURN_VALUE_IF(l_alignedTypeSize == k_initialElementStrideSize, "アライメント後の値が無効となっており、作成処理に失敗しました。", false);

	m_elementStrideSize = l_alignedTypeSize;

	const auto& l_totalSize = m_createCount * l_alignedTypeSize;

	FWK_ASSERT_RETURN_VALUE_IF(l_totalSize == UploadBuffer::k_invalidBufferSize, "作成バッファーサイズが0のためバッファの生成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_uploadBuffer.Create(a_device, l_totalSize),    "バッファの生成処理に失敗しました。",                              false);

	return true;
}