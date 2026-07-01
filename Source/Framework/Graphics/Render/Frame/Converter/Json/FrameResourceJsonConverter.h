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

		void DeserializeDynamicBuffer(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const;

		nlohmann::json SerializeDynamicBuffer(const Graphics::FrameResource& a_frameResource) const;

		static constexpr std::string_view k_dynamicBufferUploaderListJsonKey	 = "DynamicBufferUploaderList";
		static constexpr std::string_view k_dynamicBufferUploaderTypeNameJsonKey = "DynamicBufferUploaderTypeName";
		static constexpr std::string_view k_dynamicBufferUploaderJsonKey		 = "DynamicBufferUploader";

		static constexpr std::string_view k_renderGraphFrameResourceJsonkey = "RenderGraphFrameResource";
	};
}