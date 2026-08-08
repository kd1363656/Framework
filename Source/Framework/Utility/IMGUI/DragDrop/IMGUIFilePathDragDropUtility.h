#pragma once

namespace FWK::Utility
{
	template <typename Type>
	inline bool DragDropSource(const std::string_view& a_label, const Type& a_payload)
	{
		constexpr auto l_kind = TypeTrait::PTRType<Type>::k_kind;

		static_assert(l_kind != Enum::PTRKind::Unique, "std::unique_ptrはDrawDropに対応していません");

		// 生ポインタまたはshared_ptrが空の場合は送信しない
		if constexpr (l_kind == Enum::PTRKind::Raw    ||
			          l_kind == Enum::PTRKind::Shared)
		{
			if (!a_payload) { return false; }
		}

		if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) { return false; }

		if constexpr (l_kind == Enum::PTRKind::Raw    ||
			          l_kind == Enum::PTRKind::Shared ||
			          l_kind == Enum::PTRKind::Weak)
		{
			// shared_ptrなどのオブジェクトをByteCopyすると
			// shared_ptr本来のコピー処理が行われない
			// そのため送信元オブジェクトを示すPoiter値だけを
			// ImGuiPayloadへコピーする
			// a_payload本体ではなく
			// a_payloadを指すポインタ値をPayloadeへコピーする
			const bool l_isPayloadSet = ImGui::SetDragDropPayload(a_label.data(), &a_payload, sizeof(l_payloadPointer));
		}
		else
		{
			static_assert(l_kind == Enum::PTRKind::None, "未対応のPTRKindです");

			// DearImGuiはSetDragDropPayload()内部で
			// Payloadをmemcypyによって保存する
			// そのためType本体をPayloadとして渡せるのはByteCopyしても
			// C++オブジェクトとして意味が変わらない
			// TriviallyCopyableなTypeだけに限定する
			l_isPayloadSet = ImGui::SetDragDropPayload(a_label.data(), &a_payload, sizeof(Type));
		}

		ImGui::Text("%s", a_label.data());

		ImGui::EndDragDropSource();

		return l_isPayloadSet;
	}

	template <typename Type>
	inline bool DragDropTarget(const std::string_view& a_label, Type& a_outPayload)
	{
		constexpr auto l_kind = TypeTrait::PTRType<Type>::k_kind;

		static_assert(l_kind != Enum::PTRKind::Unique, "std::unique_ptrはDrawDropに対応していません");

		if (!ImGui::BeginDragDropTarget()) { return false; }

		const auto* l_payload   = ImGui::AcceptDragDropPayload(a_label.data());
		      bool  l_isDropped = false;

		if constexpr (l_kind == Enum::PTRKind::Raw    ||
			          l_kind == Enum::PTRKind::Shared ||
			          l_kind == Enum::PTRKind::Weak)
		{
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
					const auto* l_payloadPointer = *l_payloadPointerAddress;

					// 送信元オブジェクトから通常のコピー代入を行う
					// filesystem::pathやshared_ptrも
					// 正規のコピー処理になる
					a_outPayload = *l_payloadPointer;

					l_isDropped = true;
				}
			}	
		}
		else
		{
			static_assert(l_kind == Enum::PTRKind::None,      "未対応のPTRKindです");
			static_assert(std::is_trivially_copyable_v<Type>, "値型のDragDropPayloadはTriviallyCopyableである必要があります");

			if (l_payload               &&
				l_payload->IsDelivery() &&
				l_payload->Data         &&
				l_payload->DataSize == sizeof(Type))
			{
				// Source側と同様、trivially copyableなTypeなので
				// ImGui PayloadからそのままByteCopyできる。
				std::memcpy(&a_outPayload, l_payload->Data, sizeof(Type));

				l_isDropped = true;
			}
		}

		ImGui::EndDragDropTarget();
		
		return l_isDropped;
	}

	inline bool FilePathDragDropSource(const std::string_view& a_label, const std::filesystem::path& a_filePath)
	{
		if (a_filePath.empty()) { return false; }

		if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) { return false; }

		const     auto& l_nativeFilePath             = a_filePath.native();
		constexpr auto  l_nullTerminatorElementCount = Constant::k_filePathNullTerminatorElementCount;

		// std::filesystem::pathそのものはByteCopyせず、
		// Path文字列だけをImGuiPayloadへコピーする
		// SetDragDropPayload()側がPayloadデータを内部へコピーするため
		// ContentBrowser側のFilePathがLocal変数でも問題ない
		const auto& l_payloadByteSize =                          (l_nativeFilePath.size() + l_nullTerminatorElementCount) * sizeof(std::filesystem::path::value_type);
		const bool  l_isPayloadSet    = ImGui::SetDragDropPayload(a_label.data(), l_nativeFilePath.c_str(), l_payloadByteSize);
		const auto& l_fileName        = a_filePath.filename      ().string();

		ImGui::TextUnformatted  (l_fileName.c_str());
		ImGui::EndDragDropSource();

		return l_isPayloadSet;
	}

	inline bool FilePathDragDropTarget(const std::string_view& a_label, std::filesystem::path& a_outFilePath)
	{
		if (!ImGui::BeginDragDropTarget()) { return false; }

		const auto* l_payload = ImGui::AcceptDragDropPayload(a_label.data());

		bool l_isDropped = false;

		if (l_payload               &&
			l_payload->IsDelivery() &&
			l_payload->Data         &&
			l_payload->DataSize > NULL)
		{
			const auto& l_payloadByteSize   = static_cast<std::size_t>(l_payload->DataSize);
			const auto& l_valueTypeByteSize = sizeof                  (std::filesystem::path::value_type);

			// Payloadがpath::value_type単位で構成されているか確認する
			if (l_payloadByteSize % l_valueTypeByteSize == static_cast<std::size_t>(NULL))
			{
				const std::size_t l_elementCount = l_payloadByteSize / l_valueTypeByteSize;
				const auto*       l_filePathData = static_cast<const std::filesystem::path::value_type*>(l_payload->Data);

				// Source側でNull終端までPayloadへコピーしているので、
				// 最後の要素がNULLであることも確認する
				if (l_elementCount >= Constant::k_filePathNullTerminatorElementCount &&
					l_filePathData                                                   &&
					l_filePathData[l_elementCount - Constant::k_filePathNullTerminatorElementCount] == std::filesystem::path::value_type())
				{
					a_outFilePath = std::filesystem::path(l_filePathData);
				
					l_isDropped = !a_outFilePath.empty();
				}
			}
		}

		ImGui::EndDragDropTarget();

		return l_isDropped;
	}
}