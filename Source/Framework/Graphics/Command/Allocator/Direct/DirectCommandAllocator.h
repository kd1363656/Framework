#pragma once

namespace FWK::Graphics
{
	class DirectCommandAllocator final : public CommandAllocatorBase
	{
	public:

		 DirectCommandAllocator();
		~DirectCommandAllocator() override;
	};
}