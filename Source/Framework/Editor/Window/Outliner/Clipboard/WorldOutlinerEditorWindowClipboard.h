#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowClipboard final
	{
	public:

		 WorldOutlinerEditorWindowClipboard() = default;
		~WorldOutlinerEditorWindowClipboard() = default;

		void Apply(const std::vector<boost::uuids::uuid>& a_gameObjectUUIDList, const Enum::WorldOutlinerClipboardOperationType a_operationType);

		void Clear();

		bool Contains(const boost::uuids::uuid& a_gameObjectUUID) const;

		bool IsEmpty() const;

		const auto& GetREFGameObjectUUIDList() const { return m_clipboardGameObjectUUIDList; }

		auto GetVALOperationType() const { return m_operationType; }

	private:

		std::unordered_set<boost::uuids::uuid> m_clipboardGameObjectUUIDSet = {};

		std::vector<boost::uuids::uuid> m_clipboardGameObjectUUIDList = {};

		Enum::WorldOutlinerClipboardOperationType m_operationType = Enum::WorldOutlinerClipboardOperationType::Invalid;
	};
}