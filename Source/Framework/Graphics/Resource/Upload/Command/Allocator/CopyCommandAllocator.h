#pragma once

namespace FWK::Graphics
{
	class CopyCommandAllocator final : public CommandAllocatorBase
	{
	public:

		 CopyCommandAllocator();
		~CopyCommandAllocator() override;
	};
}