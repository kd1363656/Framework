#pragma once

namespace FWK::Enum
{
	enum class DirectorySynchronizationResult
	{
		NoChange,

		RequiresFolderTreeRefresh,

		RequiresFullResynchronization
	};
}