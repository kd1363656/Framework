#pragma once

namespace FWK::Graphics
{
	class DynamicBufferUploaderBase;
}

namespace FWK::Converter
{
	class DynamicBufferUploaderBaseJsonConverter final
	{
	public:
		
		 DynamicBufferUploaderBaseJsonConverter() = default;
		~DynamicBufferUploaderBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::DynamicBufferUploaderBase& a_dynamicBufferUploaderBase) const;

		nlohmann::json Serialize(const Graphics::DynamicBufferUploaderBase& a_dynamicBufferUploaderBase) const;

	private:

		static constexpr std::string_view k_createCountJsonKey = "CreateCount";
	};
}