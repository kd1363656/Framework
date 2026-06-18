#pragma once

namespace FWK::Graphics
{
	class StaticModelRecord final : public AssetRecordBase
	{
	public:

		 StaticModelRecord()		  = default;
		~StaticModelRecord() override = default;
		
		StaticModelRecord(const StaticModelRecord&)			  = delete;
		StaticModelRecord(	    StaticModelRecord&&) noexcept = default;

		StaticModelRecord& operator=(const StaticModelRecord&)			 = delete;
		StaticModelRecord& operator=(	   StaticModelRecord&&) noexcept = default;

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

		void SetModelData(Struct::StaticModelData&& a_set) { m_modelData = std::move(a_set); }

		const auto& GetREFModelData() const { return m_modelData; }

		auto& GetMutableREFModelData() { return m_modelData; }

	private:

		Struct::StaticModelData m_modelData = {};
	};
}