#pragma once

namespace FWK::Concept
{
	// EditorMenuBaseを継承したEditorWindowかどうかを確認するConcept
	template <typename Type>
	concept IsDerivedEditorMainMenuBaseConcept = IsDerivedBaseConcept<Type, Editor::EditorWindowBase>;
}