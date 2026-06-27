#pragma once

namespace FWK::TypeAlias
{
	using EditorWindowSharedFactory = GenericFactory<std::shared_ptr<FWK::Editor::EditorWindowBase>>;
}