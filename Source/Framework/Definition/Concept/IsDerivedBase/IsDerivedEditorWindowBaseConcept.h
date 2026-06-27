#pragma once

namespace FWK::Concept
{
	// EditorWindowBaseを継承したEditorWindowかどうかを確認するConcept
	template <typename Type>
	concept IsDerivedEditorWindowBaseConcept = IsDerivedBaseConcept<Type, Editor::EditorWindowBase>;
}