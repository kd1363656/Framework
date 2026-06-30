#pragma once

namespace FWK::Graphics
{
	class ConstantBufferUploaderBase;
}

namespace FWK::Converter
{
	class ConstantBufferUploaderBaseJsonConverter final
	{
	public:
		
		 ConstantBufferUploaderBaseJsonConverter() = default;
		~ConstantBufferUploaderBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::ConstantBufferUploaderBase& a_constantBufferUploaderBase) const;

		nlohmann::json Serialize(const Graphics::ConstantBufferUploaderBase& a_constantBufferUploaderBase) const;

	private:

		static constexpr std::string_view k_createConstantBufferCountJsonKey = "CreateConstantBufferCount";
	};
}