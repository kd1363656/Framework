#include "BinaryFileConverterBase.h"

FWK::Converter::BinaryFileConverterBase::BinaryFileConverterBase() : 
	m_mappedData(nullptr),

	m_fileHandle       (INVALID_HANDLE_VALUE),
	m_fileMappingHandle(nullptr),

	m_mappedDataSize(k_emptyMappedDataSize),

	m_isWritable(k_isInitialWritable)
{}
FWK::Converter::BinaryFileConverterBase::~BinaryFileConverterBase()
{
	// 最後にクラス自体が破棄されるときにも、
	// 開いたままのメモリマップ・ハンドル、ファイルハンドルを必ず解放するため
	DestroyMemoryMappedFile();
}

bool FWK::Converter::BinaryFileConverterBase::CreateReadMemoryMappedFile(const std::filesystem::path& a_filePath)
{
	// 既に別のファイルを開いていた場合に備えて前のマッピングを破棄する
	DestroyMemoryMappedFile();

	// ファイルを読み込めるかを確認
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath), "BinaryFileの読み込みに失敗しました。", false);

	// ファイルを開く
	// CreateFileW(開くファイルパス、
	//			   読み込み専用で開く指定、
	//			   他の読み込みアクセスを許可する指定、
	//			   セキュリティ属性、
	//			   既存ファイルのみ開く指定、
	//			   通常ファイルとして扱う指定、
	//			   テンプレートファイルを使用しないためnullptr);
	m_fileHandle = CreateFileW(a_filePath.c_str(),
							   GENERIC_READ,
							   FILE_SHARE_READ,
							   nullptr,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   nullptr);

	// 無効なファイルハンドルならreturn
	if (m_fileHandle == INVALID_HANDLE_VALUE)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの読み込み用ファイルハンドル作成に失敗しました。", false);
	}

	auto l_fileSizeHigh = k_fileSizeHigh;

	// GetFileSize()がINVALID_FILE_SIZEを返したときに、
	// 本当にエラーか判定するため、事前にエラー状態をリセットする
	SetLastError(NO_ERROR);

	// ファイルサイズを上位、下位32bitで取得している
	// GetFileSize(ファイルハンドル、
	//			   ファイルサイズ上位32bitの受取先);
	const auto l_fileSizeLow = GetFileSize(m_fileHandle, &l_fileSizeHigh);

	// GetFileSize()の戻り値である下位32bitがINVALID_FILE_SIZEだった場合、
	// それが本当にエラーなのか、またはファイルサイズの下位32bitがたまたま0xFFFFFFFFなのかをGetLastError()で確認する
	if (l_fileSizeLow == INVALID_FILE_SIZE)
	{
		const auto l_error = GetLastError();

		if (l_error != NO_ERROR)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("BinaryFileのファイルサイズ取得に失敗しました。", false);
		}
	}

	// ファイルサイズは下位32bit/上位32bitに分かれて返るため、
	// High側を上位32bitへ移動して64bitのサイズに戻す
	m_mappedDataSize = (static_cast<std::uint64_t>(l_fileSizeHigh) << k_highDWORDShiftBitCount) | static_cast<std::uint64_t>(l_fileSizeLow);

	if (m_mappedDataSize == k_emptyMappedDataSize)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの読み込み用ファイルサイズが0です。", false);
	}

	// ファイルをメモリ空間へ対応付けるための中間管理オブジェクトを作成する
	// CreateFileMappingW(マッピング対象のファイルハンドル、
	//					  セキュリティ属性、
	//					  読み込み専用ページとして作成する指定、
	//					  最大サイズ上位32bit(0ならファイルサイズを使用)、
	//					  最大サイズ下位32bit(0ならファイルサイズを使用)、
	//					  名前付きマッピング);
	m_fileMappingHandle = CreateFileMappingW(m_fileHandle,
										     nullptr,
										     PAGE_READONLY,
											 k_mappingMaxSizeHighUseFileSize,
											 k_mappingMaxSizeLowUseFileSize,
											 nullptr);

	if (!m_fileMappingHandle)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの読み込み用ファイルマッピング作成に失敗しました。", false);
	}

	// CreateFileMappingWで作成したマッピングオブジェクトを使って、
	// ファイル内容をこのプロセスのメモリ空間へ対応付ける
	// 成功するとファイル内容へアクセスできる先頭ポインタが帰ってくる
	// MapViewOfFile(マッピングオブジェクトのハンドル、
	//				 読み込み専用でビューを作成する指定、
	//				 ファイルオフセット上位32bit(0の場合ファイル先頭から読む)、
	//				 ファイルオフセット下位32bit(0の場合ファイル先頭から読む)、
	//				 マップするサイズ、0ならファイル全体);
	m_mappedData = static_cast<std::uint8_t*>(MapViewOfFile(m_fileMappingHandle,
															FILE_MAP_READ,
															k_viewFileOffsetHighFromBegin,
														    k_viewFileOffsetLowFromBegin,
															k_mapEntireFileSize));

	if (!m_mappedData)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの読み込み用メモリマップに失敗しました。", false);
	}

	m_isWritable = k_isReadOnlyMappedFile;

	return true;
}
bool FWK::Converter::BinaryFileConverterBase::CreateWriteMemoryMappedFile(const std::filesystem::path& a_filePath, const std::uint64_t& a_fileSize)
{
	// 既に別のファイルを開いていた場合に備えて前のマッピングを破棄する
	DestroyMemoryMappedFile();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_filePath.empty(),				  "BinaryFileの書き込み用ファイルパスが空です。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_fileSize == k_emptyWriteFileSize, "BinaryFileの書き込みサイズが0です。",		      false);

	// createFileWは存在しない親フォルダまでは作成してくれないため、
	// 書き込み先ファイルの親フォルダが指定されている場合は、
	// ファイル作成までに途中のフォルダも含めて作成しておく
	if (const auto& l_parentPath = a_filePath.parent_path();
		!l_parentPath.empty())
	{
		std::filesystem::create_directories(l_parentPath);
	}

	// ファイルを作成する
	// CreateFileW(作成するファイルパス、
	//			   読み込みと書き込みを行う指定、
	//			   他のアクセスを許可しない指定、
	//			   セキュリティ属性、
	//			   常に新規作成または上書きする指定、
	//			   通常ファイルとして扱う指定、
	//			   テンプレートファイルを使用しないためnullptr);
	m_fileHandle = CreateFileW(a_filePath.c_str(),
							   GENERIC_READ | GENERIC_WRITE,
							   k_noFileShareMode,
							   nullptr,
							   CREATE_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL,
							   nullptr);

	if (m_fileHandle == INVALID_HANDLE_VALUE)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの書き込み用ファイルハンドル作成に失敗しました。", false);
	}

	const auto l_fileSizeLow  = static_cast<DWORD>(a_fileSize);
		  auto l_fileSizeHigh = static_cast<LONG> (a_fileSize >> k_highDWORDShiftBitCount);

	SetLastError(NO_ERROR);

	// MemoryMappedFileで書き込むには、先に書き込み先ファイルのサイズを確保しておく必要がある、
	// SetFilePointerでファイル先頭からa_fileSizeバイトの位置へ移動し、
	// このとあのSetEndOfFileでその位置をファイル終端として確定させる。
	// SetFilePointerの戻り値がINVALID_SET_FILE_POINTERでも、
	// 移動後の下位32bitがたまたま0xFFFFFFFFの可能性があるため、GetLastError()で本島に失敗したか確認する。
	// SetFilePointer(ファイルハンドル、
	//				  移動距離下位32bit、
	//				  移動距離上位32bit、
	//				  ファイル先頭から移動する指定);
	if (const auto l_setFilePointerResult = SetFilePointer(m_fileHandle,
													       l_fileSizeLow,
													       &l_fileSizeHigh,
													       FILE_BEGIN);
		l_setFilePointerResult == INVALID_SET_FILE_POINTER)
	{
		const auto l_error = GetLastError();

		if (l_error != NO_ERROR)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("BinaryFileの書き込み用ファイルサイズ指定位置の移動に失敗しました。", false);
		}
	}

	// 現在のファイル位置を終端として、MemoryMappedFileで書き込むためのファイルサイズを確保する
	// SetEndOfFile(ファイルハンドル);
	if (!SetEndOfFile(m_fileHandle))
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの書き込み用ファイルサイズ指定位置の移動に失敗しました。", false);
	}

	m_mappedDataSize = a_fileSize;
	
	// ファイルをメモリ空間へ対応付けるための中間管理オブジェクトを作成する
	// この時点ではまだファイル内容のポインタは取得していない
	// CreateFileMappingW(マッピング対象のファイルハンドル、
	//					  セキュリティ属性、
	//					  読み書き可能ページとして作成する指定、
	//					  最大サイズ上位32bit、0ならファイルサイズを使用、
	//					  最大サイズ下位32bit、0ならファイルサイズを使用、
	//					  名前付きマッピング);
	m_fileMappingHandle = CreateFileMappingW(m_fileHandle,
											 nullptr,
											 PAGE_READWRITE,
											 k_mappingMaxSizeHighUseFileSize,
										     k_mappingMaxSizeLowUseFileSize,
											 nullptr);

	if (!m_fileMappingHandle)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの書き込み用ファイルマッピング作成に失敗しました。", false);
	}

	// CreateFileMappingWで作成したマッピングオブジェクトを使って、
	// ファイル内容をこのプロセスのメモリ空間へ対応付ける
	// 成功するとファイル内容へアクセスできる先頭ポインタが帰ってくる
	// MapViewOfFile(マッピングオブジェクトのハンドル、
	//				 読み書き可能でビューを作成する指定、
	//				 ファイルオフセット上位32bit(0の場合ファイル先頭から書き込む指定)、
	//				 ファイルオフセット下位32bit(0の場合ファイル先頭から書き込む指定)、
	//				 マップするサイズ、0ならファイル全体);
	m_mappedData = static_cast<std::uint8_t*>(MapViewOfFile(m_fileMappingHandle,
														    FILE_MAP_READ | FILE_MAP_WRITE,
															k_viewFileOffsetHighFromBegin,
															k_viewFileOffsetLowFromBegin,
															k_mapEntireFileSize));

	if (!m_mappedData)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("BinaryFileの書き込み用メモリマップに失敗しました。", false);
	}

	m_isWritable = k_isWriteMappedFile;

	return true;
}

void FWK::Converter::BinaryFileConverterBase::DestroyMemoryMappedFile()
{
	if (m_mappedData)
	{
		if (m_isWritable)
		{
			// 書き込み用にマップしていた場合は、
			// メモリ上に書き込んだ内容をファイルへ反映させる
			// FlushViewOfFile(書き込んだメモリマップ先頭アドレス、
			//				   フラッシュするバイト数、0ならビュー全体);
			FlushViewOfFile(m_mappedData, k_flushEntireViewSize);
		}

		// MapViewOfFileで取得したメモリ上のビューを解除する
		// これ以降、m_mappedDataがさしていたメモリ領域にはアクセスできない
		// UnmapViewOfFile(マップ済みビューの先頭アドレス);
		UnmapViewOfFile(m_mappedData);

		m_mappedData = nullptr;
	}

	if (m_fileMappingHandle)
	{
		// CreateFileMappingWで作成したマッピングオブジェクトのハンドルを閉じる
		// ビューを解放した後に、マッピングオブジェクトのハンドルを解放する
		// CloseHandle(閉じるWinAPIハンドル);
		CloseHandle(m_fileMappingHandle);

		m_fileMappingHandle = nullptr;
	}

	if (m_fileHandle != INVALID_HANDLE_VALUE)
	{
		// CreateFileWで作成したファイルハンドルを閉じる
		// マッピングオブジェクトを解放した後に、元のファイルハンドルを解放する
		// CloseHandle(閉じるWinAPIハンドル);
		CloseHandle(m_fileHandle);

		m_fileHandle = INVALID_HANDLE_VALUE;
	}

	// 次に別のファイルを開けるように、管理用の状態を初期状態へ戻す
	m_mappedDataSize = k_emptyMappedDataSize;
	m_isWritable     = k_isInitialWritable;
}

void FWK::Converter::BinaryFileConverterBase::ReadWStringBinaryData(const std::uint64_t& a_stringBinaryFileSize, 
																	const std::uint8_t*  a_readData,
																		  std::wstring&  a_string,
																		  std::uint64_t& a_readOffset) const
{
	if (a_stringBinaryFileSize == k_emptyReadDataSize)
	{
		// 文字列のバイナリサイズが0の場合は、から文字列として扱う
		a_string.clear();

		return;
	}

	// バイナリ上では文字列サイズをバイト数で管理しているため、
	// wchar_tのサイズで割って、std::wstringとsh知恵必要な文字数に戻す
	const auto& l_stringLength = a_stringBinaryFileSize / sizeof(wchar_t);

	// 読み込み先のstd::wstringに、読み込む文字列数分の領域を確保する
	a_string.resize(l_stringLength);

	// メモリマップされたバイナリデータから、std::wstringの文字データを読み込む
	// ReadBinaryData内で、読み込んだバイト数分だけa_readOffsetが進む
	ReadBinaryData(l_stringLength,
				   a_readData, 
				   a_readOffset,
				   a_string.data());
}
void FWK::Converter::BinaryFileConverterBase::WriteWStringBinaryData(const std::wstring& a_string, std::uint64_t& a_writeOffset, std::uint8_t* a_writeData) const
{
	if (a_string.empty()) { return; }

	// std::wstringの文字データだけを書き込む
	// 文字列のサイズ自体はここでは書き込まないため、
	// 呼び出し側で先にヘッダーなどへCalculateWStringBinaryFileSize()の結果を保存しておく
	// WriteBinaryData内で、書き込んだバイト数分だけa_writeOffsetが進む
	WriteBinaryData(a_string.size(), a_string.data(), a_writeOffset, a_writeData);
}

std::uint64_t FWK::Converter::BinaryFileConverterBase::CalculateWStringBinaryFileSize(const std::wstring& a_string) const
{
	// std::wstringの文字数を、バイナリファイルへ書き込むバイト数に変換する
	// 終端文字は保存しないため、size()分のwchar_tだけをファイルサイズとして計算する
	return sizeof(wchar_t) * a_string.size();
}