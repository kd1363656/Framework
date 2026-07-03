#pragma once

namespace FWK::Editor
{
	class FileMenuEditor final : public EditorMenuBase
	{
	public:

				 FileMenuEditor()          = default;
		virtual ~FileMenuEditor() override = default;

		void Init() override;
		void Draw() override;

		FWK_DEFINE_TYPE_INFO(EditorMenuBase, EditorMenuBase)
	};
}