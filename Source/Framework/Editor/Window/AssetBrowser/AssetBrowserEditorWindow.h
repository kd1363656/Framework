#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 AssetBrowserEditorWindow()          = default;
		~AssetBrowserEditorWindow() override = default;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		void Draw() override;

		nlohmann::json Serialize() override;

	private:

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		static constexpr std::string_view k_editorName = "アセットブラウザー";

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)