#pragma once

namespace FWK::Graphics
{
	class ComputeCommandList final : public CommandListBase
	{
	public:

		 ComputeCommandList();
		~ComputeCommandList() override;
	};
}