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

		bool IsUpdatedSourceFile(const std::filesystem::path& a_sourceFilePath, const std::filesystem::path& a_binaryFilePath) const;
		
		std::filesystem::path CreateAssetFilePath(const std::filesystem::path& a_filePath) const;

		bool CreateReadMemoryMappedFile (const std::filesystem::path& a_filePath);
		bool CreateWriteMemoryMappedFile(const std::filesystem::path& a_filePath, const std::uint64_t& a_fileSize);

		void DestroyMemoryMappedFile();

		const auto& GetREFMappedDataSize() const { return m_mappedDataSize; }

		template <typename Type>
		bool TryReadBinaryData(const std::uint64_t& a_readDataCount, std::uint64_t& a_memoryReadOffet, Type* a_destinationData) const
		{
			if (a_readDataCount == k_emptyReadDataSize) { return true; }

			if (!m_mappedData)      { return false; }
			if (!a_destinationData) { return false; }

			// バイナリーデータサイズを計算してからデータを読み取る
			const auto& l_binaryDataSize = CalculateBinaryDataSize<Type>(a_readDataCount);

			if (!CanReadBinaryData(a_memoryReadOffet, l_binaryDataSize)) { return false; }

			ReadBinaryData(a_readDataCount, a_memoryReadOffet, a_destinationData);

			return true;
		}

		template <typename Type>
		bool TryReadBinaryDataList(const std::uint64_t& a_readDataCount, std::vector<Type>& a_destinationDataList, std::uint64_t& a_memoryReadOffset) const
		{
			// 前読み込んでたデータがあればデータに不都合が生じる可能性があるためクリア処理
			a_destinationDataList.clear();

			if (a_readDataCount == k_emptyReadDataSize) { return true; }

			// 読み込むサイズ分リストを確保
			a_destinationDataList.resize(a_readDataCount);

			if (!TryReadBinaryData(a_readDataCount, a_memoryReadOffset, a_destinationDataList.data())) 
			{
				a_destinationDataList.clear();

				return false;
			}

			return true;
		}

		template <typename Type>
		bool TryReadSingleBinaryData(Type& a_destinationData, std::uint64_t& a_memoryReadOffset) const
		{
			return TryReadBinaryData(k_singleBinaryElementCount, a_memoryReadOffset, &a_destinationData);
		}

		template <typename Type>
		void WriteBinaryData(const std::uint64_t& a_writeDataCount, const Type* a_sourceData, std::uint64_t& a_memoryWriteOffset) const
		{
			// 書き込みデータの型サイズと個数から、実際にコピーするバイト数を計算する
			const auto l_writeDataSize = CalculateBinaryDataSize<Type>(a_writeDataCount);

			// 書き込むバイト数が0の場合は、何もせずに終了する
			if (l_writeDataSize == k_emptyWriteDataSize) { return; }

			FWK_ASSERT_RETURN_IF(!a_sourceData, "書き込み元データがnullptrです。");
			FWK_ASSERT_RETURN_IF(!m_mappedData, "書き込み先データがnullptrです。");

			// 書き込み先のメモリマップ領域の現在位置へ、
			// 指定された型と個数分のデータを書き込む
			std::memcpy(m_mappedData + a_memoryWriteOffset, a_sourceData, l_writeDataSize);

			// 次のデータを続けて書けるように、書き込んだバイト数分だけオフセットを進める
			a_memoryWriteOffset += l_writeDataSize;
		}

		bool TryReadWStringBinaryData(const std::uint64_t& a_wStringBinaryFileSize, std::wstring& a_destinationString, std::uint64_t& a_memoryReadOffset) const;
		
		void WriteWStringBinaryData(const std::wstring& a_wString, std::uint64_t& a_memoryWriteOffset) const;
		void WriteStringBinaryData (const std::string&  a_string,  std::uint64_t& a_memoryWriteOffset) const;

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

		bool CanReadBinaryData(const std::uint64_t& a_memoryReadOffset, const std::uint64_t& a_readDataSize) const;

		template <typename Type>
		void ReadBinaryData(const std::uint64_t& a_readDataCount, std::uint64_t& a_memoryReadOffset, Type* a_destinationData) const
		{
			// 読み込むデータの型サイズと個数から、実際にコピーするバイト数を計算する
			const auto l_readDataSize = CalculateBinaryDataSize<Type>(a_readDataCount);

			// 読み込むバイト数が0の場合は、何もせずに終了する
			if (l_readDataSize == k_emptyReadDataSize) { return; }

			FWK_ASSERT_RETURN_IF(!m_mappedData,	    "読み込み元データがnullptrです。");
			FWK_ASSERT_RETURN_IF(!a_destinationData, "読み込み先データがnullptrです。");

			// メモリマップされたバイナリデータの現在位置から、
			// 指定された型と個数分のデータを読み込み先へコピーする
			std::memcpy(a_destinationData, m_mappedData + a_memoryReadOffset, l_readDataSize);

			// 次のデータを続けて読めるように、読み込んだバイト数分だけオフセットを進める
			a_memoryReadOffset += l_readDataSize;
		}

		void ReadWStringBinaryData(const std::uint64_t& a_wStringBinaryFileSize, std::wstring& a_wString, std::uint64_t& a_memoryReadOffset) const;
		void ReadStringBinaryData (const std::uint64_t& a_stringBinaryFileSize,  std::string&  a_string,  std::uint64_t& a_memoryReadOffset) const;

		static constexpr SIZE_T k_mapEntireFileSize   = 0ULL;
		static constexpr SIZE_T k_flushEntireViewSize = 0ULL;

		static constexpr std::uint64_t k_initialMemoryReadOffset  = 0ULL;
		static constexpr std::uint64_t k_initialMemoryWriteOffset = 0ULL;

		static constexpr std::uint64_t k_emptyMappedDataSize = 0ULL;
		static constexpr std::uint64_t k_emptyWriteFileSize  = 0ULL;

		static constexpr std::uint64_t k_emptyReadDataSize  = 0ULL;
		static constexpr std::uint64_t k_emptyWriteDataSize = 0ULL;

		static constexpr std::uint64_t k_singleBinaryElementCount = 1ULL;

		static constexpr DWORD k_fileSizeHigh				   = 0UL;
		static constexpr DWORD k_mappingMaxSizeHighUseFileSize = 0UL;
		static constexpr DWORD k_mappingMaxSizeLowUseFileSize  = 0UL;
		static constexpr DWORD k_viewFileOffsetHighFromBegin   = 0UL;
		static constexpr DWORD k_viewFileOffsetLowFromBegin    = 0UL;
		static constexpr DWORD k_noFileShareMode			       = 0UL;

		static constexpr std::uint32_t k_highDWORDShiftBitCount = 32U;

		static constexpr bool k_isInitialWritable    = false;
		static constexpr bool k_isReadOnlyMappedFile = false;
		static constexpr bool k_isWriteMappedFile    = true;

		HANDLE m_fileHandle;
		HANDLE m_fileMappingHandle;

		std::uint8_t* m_mappedData;

		std::uint64_t m_mappedDataSize;

		bool m_isWritable = k_isInitialWritable;
	};
}