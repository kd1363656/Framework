#pragma once

namespace FWK::Utility
{
	template <typename Type>
	inline bool DragDropSource(const std::string_view& a_label, const Type& a_payload)
	{
		constexpr auto l_kind = TypeTrait::PTRType<Type>::k_kind;

		static_assert(l_kind != TypeTrait::PTRKind::Unique, "std::unique_ptrはDrawDropに対応していません");

		// 生ポインタまたはshared_ptrが空の場合は送信しない
		if constexpr (l_kind == TypeTrait::PTRKind::Raw    ||
			          l_kind == TypeTrait::PTRKind::Shared)
		{
			if (!a_payload) { return false; }
		}

		if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) { return false; }

		// a_payload本体ではなく
		// a_payloadを指すポインタ値をPayloadeへコピーする
		const Type* l_payloadPointer = &a_payload;

		const bool l_isPayloadSet = ImGui::SetDragDropPayload(a_label.data(), &l_payloadPointer, sizeof(l_payloadPointer));

		ImGui::Text("%s", a_label.data());

		ImGui::EndDragDropSource();

		return true;
	}

	template <typename Type>
	inline bool DragDropTarget(const std::string_view& a_label, Type& a_outPayload)
	{
		constexpr auto l_kind = TypeTrait::PTRType<Type>::k_kind;

		static_assert(l_kind != TypeTrait::PTRKind::Unique, "std::unique_ptrはDrawDropに対応していません");

		if (!ImGui::BeginDragDropTarget()) { return false; }

		const auto* l_payload   = ImGui::AcceptDragDropPayload(a_label.data());
		      bool  l_isDropped = false;

		if (l_payload               &&
			l_payload->IsDelivery() &&
			l_payload->Data         &&
			l_payload->DataSize == sizeof(const Type*))
		{
			// PayloadにはconstType*の値が格納されているため、
			// DataはconstType*を指し示すPointerとして解釈する
			const auto* const* l_payloadPointerAddress = static_cast<const Type* const*>(l_payload->Data);

			if (l_payloadPointerAddress && 
				*l_payloadPointerAddress)
			{
				const Type* l_payloadPointer = *l_payloadPointerAddress;

				// 送信元オブジェクトから通常のコピー代入を行う
				// filesystem::pathやshared_ptrも
				// 正規のコピー処理になる
				a_outPayload = *l_payloadPointer;

				l_isDropped = true;
			}
		}

		ImGui::EndDragDropTarget();
		
		return l_isDropped;
	}
}