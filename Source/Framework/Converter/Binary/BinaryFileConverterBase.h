#pragma once

namespace FWK::Converter
{
	// メモリマップドファイルを使って、バイナリーファイルの読み書きを行うための基底クラス
	// メモリマップドファイルは通常のファイル読み込みのようなコピーをせず
	// ファイルを開きメモリ空間にファイル内容を対応付けるため高速
	// コピーすると同じハンドルと同じマップ済みポインタを複数インスタンスが持つ可能性があるのでコピー、ムーブ禁止
	class BinaryFileConverterBase
	{
	public:

				 BinaryFileConverterBase();
		virtual ~BinaryFileConverterBase();

		BinaryFileConverterBase(const BinaryFileConverterBase&)			  = delete;
		BinaryFileConverterBase(	  BinaryFileConverterBase&&) noexcept = delete;

		BinaryFileConverterBase& operator=(const BinaryFileConverterBase&)			 = delete;
		BinaryFileConverterBase& operator=(	     BinaryFileConverterBase&&) noexcept = delete;

	protected:

		bool CreateReadMemoryMappedFile (const std::filesystem::path& a_filePath);
		bool CreateWriteMemoryMappedFile(const std::filesystem::path& a_filePath, const std::uint64_t& a_fileSize);

		void DestroyMemoryMappedFile();

		const auto& GetREFMappedDataSize() const { return m_mappedDataSize; }

		const std::uint8_t* GetPTRMappedData() const { return m_mappedData; }

		std::uint8_t* GetMutablePTRMappedData() { return m_mappedData; }

		template <typename Type>
		void ReadBinaryData(const std::uint64_t& a_readDataCount,
							const std::uint8_t*  a_mappedData,
							      std::uint64_t& a_memoryReadOffset,
								  Type*          a_destinationData) const
		{
			// 読み込むデータの型サイズと個数から、実際にコピーするバイト数を計算する
			const auto l_readDataSize = sizeof(Type) * a_readDataCount;

			// 読み込むバイト数が0の場合は、何もせずに終了する
			if (l_readDataSize == k_emptyReadDataSize) { return; }

			FWK_ASSERT_RETURN_IF_FAILED(!a_mappedData,	    "読み込み元データがnullptrです。");
			FWK_ASSERT_RETURN_IF_FAILED(!a_destinationData, "読み込み先データがnullptrです。");

			// メモリマップされたバイナリデータの現在位置から、
			// 指定された型と個数分のデータを読み込み先へコピーする
			std::memcpy(a_destinationData, a_mappedData + a_memoryReadOffset, l_readDataSize);

			// 次のデータを続けて読めるように、読み込んだバイト数分だけオフセットを進める
			a_memoryReadOffset += l_readDataSize;
		}

		template <typename Type>
		void WriteBinaryData(const std::uint64_t& a_writeDataCount,
							 const Type*		  a_sourceData,
								   std::uint64_t& a_memoryWriteOffset,
								   std::uint8_t*  a_mappedData) const
		{
			// 書き込みデータの型サイズと個数から、実際にコピーするバイト数を計算する
			const auto l_writeDataSize = sizeof(Type) * a_writeDataCount;

			// 書き込むバイト数が0の場合は、何もせずに終了する
			if (l_writeDataSize == k_emptyWriteDataSize) { return; }

			FWK_ASSERT_RETURN_IF_FAILED(!a_sourceData, "書き込み元データがnullptrです。");
			FWK_ASSERT_RETURN_IF_FAILED(!a_mappedData, "書き込み先データがnullptrです。");

			// 書き込み先のメモリマップ領域の現在位置へ、
			// 指定された型と個数分のデータを書き込む
			std::memcpy(a_mappedData + a_memoryWriteOffset, a_sourceData, l_writeDataSize);

			// 次のデータを続けて書けるように、書き込んだバイト数分だけオフセットを進める
			a_memoryWriteOffset += l_writeDataSize;
		}

		void ReadWStringBinaryData(const std::uint64_t& a_wStringBinaryFileSize,
								   const std::uint8_t*  a_readData,
										 std::wstring&  a_wString,
										 std::uint64_t& a_memoryReadOffset) const;

		void WriteWStringBinaryData(const std::wstring& a_wString, std::uint64_t& a_memoryWriteOffset, std::uint8_t* a_writeData) const;

		void ReadStringBinaryData(const std::uint64_t& a_stringBinaryFileSize,
								  const std::uint8_t*  a_readData,
										std::string&   a_string,
										std::uint64_t& a_memoryReadOffset) const;

		void WriteStringBinaryData(const std::string& a_string, std::uint64_t& a_memoryWriteOffset, std::uint8_t* a_writeData) const;

		template <typename Type>
		std::uint64_t CalculateBinaryDataSize(const std::uint64_t& a_dataCount) const
		{
			return sizeof(Type) * a_dataCount;
		}

		std::uint64_t CalculateWStringBinaryFileSize(const std::wstring& a_wString) const;
		std::uint64_t CalculateStringBinaryFileSize (const std::string&  a_string)   const;

		static constexpr auto& GetREFInitialMemoryReadOffset () { return k_initialMemoryReadOffset; }
		static constexpr auto& GetREFInitialMemoryWriteOffset() { return k_initialMemoryWriteOffset; }

		static constexpr auto GetREFSingleBinaryElementCount() { return k_singleBinaryElementCount; }

	private:

		static constexpr std::uint64_t k_initialMemoryReadOffset  = 0ULL;
		static constexpr std::uint64_t k_initialMemoryWriteOffset = 0ULL;

		static constexpr std::uint64_t k_emptyMappedDataSize = 0ULL;
		static constexpr std::uint64_t k_emptyWriteFileSize  = 0ULL;

		static constexpr std::uint64_t k_emptyReadDataSize  = 0ULL;
		static constexpr std::uint64_t k_emptyWriteDataSize = 0ULL;

		static constexpr std::uint64_t k_singleBinaryElementCount = 1ULL;

		static constexpr SIZE_T k_mapEntireFileSize   = 0ULL;
		static constexpr SIZE_T k_flushEntireViewSize = 0ULL;
		
		static constexpr std::uint32_t k_highDWORDShiftBitCount = 32U;

		static constexpr DWORD k_fileSizeHigh				   = 0UL;
		static constexpr DWORD k_mappingMaxSizeHighUseFileSize = 0UL;
		static constexpr DWORD k_mappingMaxSizeLowUseFileSize  = 0UL;
		static constexpr DWORD k_viewFileOffsetHighFromBegin   = 0UL;
		static constexpr DWORD k_viewFileOffsetLowFromBegin    = 0UL;
		static constexpr DWORD k_noFileShareMode			   = 0UL;

		static constexpr bool k_isInitialWritable    = false;
		static constexpr bool k_isReadOnlyMappedFile = false;
		static constexpr bool k_isWriteMappedFile    = true;

		std::uint8_t* m_mappedData;

		HANDLE m_fileHandle;
		HANDLE m_fileMappingHandle;

		std::uint64_t m_mappedDataSize;

		bool m_isWritable = k_isInitialWritable;
	};
}