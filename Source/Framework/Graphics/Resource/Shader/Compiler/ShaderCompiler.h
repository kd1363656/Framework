#pragma once

namespace FWK::Graphics
{
	class ShaderCompiler
	{
	public:

		 ShaderCompiler() = default;
		~ShaderCompiler() = default;

		bool Create();

		// コンパイル対象のシェーダーを格納するIDxcBlobを引数にしているのは
		// しっかりコンパイルできたかどうかを"bool"で確認するため
		// HLSLファイルを読み込み、指定エントリーポイントとシェーダーモデルでコンパイルし、
		// DXILバイトコードを出力Blobとして返
		TypeAlias::ComPtr<IDxcBlob> LoadBinaryFromFile(const std::wstring& a_filePath) const;

	private:

		TypeAlias::ComPtr<IDxcUtils>     m_dxcUtils    = nullptr;
		TypeAlias::ComPtr<IDxcCompiler3> m_dxcCompiler = nullptr;
	};
}