#pragma once

namespace FWK::Graphics
{
	class FrameResource;
}

namespace FWK::Converter
{
	class FrameResourceJsonConverter
	{
	public:

		 FrameResourceJsonConverter() = default;
		~FrameResourceJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const;

		nlohmann::json Serialize(const Graphics::FrameResource& a_frameResource) const;

	private:

		void DeserializeConstantBuffer(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const;

		nlohmann::json SerializeConstantBuffer(const Graphics::FrameResource& a_frameResource) const;

		static constexpr std::string_view k_constantBufferUploaderListJsonKey	    = "ConstantBufferUploaderList";
		static constexpr std::string_view k_constantBufferUploaderTypeNameJsonKey   = "ConstantBufferUploaderTypeName";
		static constexpr std::string_view k_constantBufferUploaderJsonKey		    = "ConstantBufferUploader";

		static constexpr std::string_view k_renderGraphFrameResourceJsonkey = "RenderGraphFrameResource";
	};
}