#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class PipelineStateBase
	{
	public:

				 PipelineStateBase() = default;
		virtual ~PipelineStateBase() = default;

		virtual void Deserialize(const nlohmann::json& a_rootJson);
		virtual bool Create     (const Device&		   a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) = 0;

		virtual nlohmann::json Serialize() const;

		void SetUseRootSignatureType(const Enum::RootSignatureType a_set) { m_useRootSignatureType = a_set; }

		const auto& GetREFUseRootSignature() const { return m_useRootSignature; }

		const auto& GetREFPipelineState() const { return m_pipelineState; }

		auto GetVALUseRootSignatureType() const { return m_useRootSignatureType; }

	protected:

		void PrepareCommonPipelineStateCreate(const Device& a_device, const Renderer& a_renderer);

		void SetUseRootSignature(const std::weak_ptr<RootSignature>& a_set) { m_useRootSignature = a_set; }

		D3D12_SHADER_BYTECODE FetchShaderByteCode(const Shader& a_shader) const;

		auto& GetMutableREFPipelineState() { return m_pipelineState; }

	private:
		
		TypeAlias::ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;

		std::weak_ptr<RootSignature> m_useRootSignature = {};

		Enum::RootSignatureType m_useRootSignatureType = Enum::RootSignatureType::Invalid;

		Converter::PipelineStateBaseJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO_ROOT(PipelineStateBase)
	};
}