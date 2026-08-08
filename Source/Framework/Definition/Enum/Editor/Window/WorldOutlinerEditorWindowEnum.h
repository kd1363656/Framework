#pragma once

namespace FWK::Enum
{
	enum class GameObjectRangeSelectionState 
	{
		None,
		BeforeRange,
		Selecting,
		Completed
	};

	enum class GameObjectHierarchyChangeRequestType
	{
		None,
		ApplyParent,
		Unparent
	};
}