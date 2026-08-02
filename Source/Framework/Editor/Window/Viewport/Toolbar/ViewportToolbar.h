#pragma once

namespace FWK::Editor
{
	class ViewportToolbar final
	{
	public:

		 ViewportToolbar() = default;
		~ViewportToolbar() = default;

		void Draw();

	private:

		void DrawDisplayOptionsButton() const;
		void DrawDisplayOptionsPopup () const;
		void DrawCollisionMenuItem   () const;

		static constexpr std::string_view k_toolbarChildID         = "##ViewportToolbar";
		static constexpr std::string_view k_displayOptionsPopupID  = "##SceneViewDisplayOptionsPopup";
		static constexpr std::string_view k_collisionMenuItemLabel = "コリジョン";

		static constexpr float k_toolbarHeight     = 36.0F;
		static constexpr float k_toolbarButtonSize = 28.0F;
	};
}