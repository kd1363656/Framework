#pragma once

namespace FWK::Graphics
{
	class ComputeCommandAllocator final : public CommandAllocatorBase
	{
	public:

		 ComputeCommandAllocator();
		~ComputeCommandAllocator() override;
	};
}