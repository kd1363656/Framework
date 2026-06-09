#include "ShaderCompiler.h"

bool FWK::Graphics::ShaderCompiler::Create()
{
	// 今回はDxcUtilsを作成している。
	// DxcUtilsはファイル読み込みや文字コード変換、include処理補助など、
	// DXCを使う周辺処理で使う便利機能用オブジェクト
	// DXC(DX Shader Compiler)関連のCOMオブジェクト作成する関数
	// DxcCreateInstance(何を作るのかを表すCLSID、
	//					 受け取りたいCOMインターフェース型のID、
	//					 作成結果のポインタを書き込むアドレス);
	auto l_hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_dxcUtils.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "DxcUtilsの作成に失敗しており、作成処理に失敗しました。", false);

	// DXC本体のコンパイラオブジェクトを作成する
	// CLSID_DxcCompilerは「シェーダーをコンパイルする本体」を表す識別子。
	// これを作成することで、HLSLコードをDXILへコンパイルできるようになる。
	l_hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_dxcCompiler.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "DxcCompilerの作成に失敗しており、作成処理に失敗しました。", false);

	return true;
}

FWK::TypeAlias::ComPtr<IDxcBlob> FWK::Graphics::ShaderCompiler::LoadBinaryFromFile(const std::wstring& a_filePath) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_dxcUtils, "DxcUtilsの作成がされておらず、シェーダーバイナリファイルの読み込みに失敗しました。", nullptr);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_filePath.empty(), "ファイルパスが空文字列のため、シェーダーバイナリファイルの読み込みに失敗しました。", nullptr);

	TypeAlias::ComPtr<IDxcBlobEncoding> l_binaryBlob = nullptr;

	// LoadFile(読み込むファイルパス、
	//			文字コード指定(nullptrなら自動又はバイナリとして扱う)、
	//			読み込んだ結果を書き込むIDxcBlobEncodingの受取先);
	// .csoはテキストではなくDXILバイトコードなので、
	// ここではコンパイルせず、そのままBlobとして読み込む
	auto l_hr = m_dxcUtils->LoadFile(a_filePath.c_str(), nullptr, l_binaryBlob.ReleaseAndGetAddressOf());

	if (FAILED(l_hr))
	{
#if defined(_DEBUG)
		OutputDebugStringA("シェーダーバイナリファイルの読み込みに失敗しました。");
#endif
		return nullptr;
	}

	TypeAlias::ComPtr<IDxcBlob> l_dxcBlob = nullptr;

	// IDxcBlobEncodingはIDxcBlobを継承しているため、
	// PSO作成で使うD3D12_SHADER_BYTECODE用のIDxcBlobとして扱えるように変換する。
	l_hr = l_binaryBlob.As(&l_dxcBlob);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "IDxcBlobEncodingからIDxcBlobへの変換に失敗しており、バイナリファイルの読み込みに失敗しました。", nullptr);

	return l_dxcBlob;
}