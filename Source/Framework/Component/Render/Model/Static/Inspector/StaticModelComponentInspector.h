#pragma once

namespace FWK
{
	class StaticModelComponent;
}

namespace FWK
{
	class StaticModelComponentInspector final
	{
	public:

		 StaticModelComponentInspector() = default;
		~StaticModelComponentInspector() = default;

		void EditInspector(StaticModelComponent& a_staticModelComponent) const;

	private:

		static constexpr std::string_view k_drawRequestDataStrategySelectorLabel = "描画シェーダー使用是非チェックボックス";
		static constexpr std::string_view k_fbxFilePathSelectorLabel             = "FBXファイルドロップエリア";
	};
}