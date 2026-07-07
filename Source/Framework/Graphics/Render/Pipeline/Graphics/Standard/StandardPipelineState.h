#pragma once

namespace FWK::Graphics
{
	class StandardPipelineState final : public GraphicsPipelineStateBase
	{
	public:

		 StandardPipelineState()          = default;
		~StandardPipelineState() override = default;

		void Deserialize(const nlohmann::json& a_rootJson)																	 override;
		bool Create     (const Device&         a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) override;

		nlohmann::json Serialize() const override;

		void ClearInputLayout();

		void AddInputElementDesc(const Struct::StandardPipelineInputElement& a_inputElement);

		const auto& GetREFInputElementList    () const { return m_inputElementList; }

		const auto& GetREFVertexShader  () const { return m_vertexShader; }
		const auto& GetREFHullShader    () const { return m_hullShader; }
		const auto& GetREFDomainShader  () const { return m_domainShader; }
		const auto& GetREFGeometryShader() const { return m_geometryShader; }
		const auto& GetREFPixelShader   () const { return m_pixelShader; }

		auto& GetMutableREFVertexShader  () { return m_vertexShader; }
		auto& GetMutableREFHullShader    () { return m_hullShader; }
		auto& GetMutableREFDomainShader  () { return m_domainShader; }
		auto& GetMutableREFGeometryShader() { return m_geometryShader; }
		auto& GetMutableREFPixelShader   () { return m_pixelShader; }

	private:

		void BuildInputElementDescList();

		Shader m_vertexShader = {};

		std::vector<D3D12_INPUT_ELEMENT_DESC>			  m_inputElementDescList = {};
		std::vector<Struct::StandardPipelineInputElement> m_inputElementList     = {};

		std::shared_ptr<Shader> m_hullShader     = nullptr;
		std::shared_ptr<Shader> m_domainShader   = nullptr;
		std::shared_ptr<Shader> m_geometryShader = nullptr;
		std::shared_ptr<Shader> m_pixelShader    = nullptr;

		Converter::StandardPipelineStateJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(StandardPipelineState, GraphicsPipelineStateBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::PipelineStateSharedFactory, FWK::Graphics::StandardPipelineState)