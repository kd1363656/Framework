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

		void SetModelData(Struct::ModelData&& a_set) { m_modelData = std::move(a_set); }

		Struct::ModelData& GetREFModelData() { return m_modelData; }

		const Struct::ModelData& GetREFModelData() const { return m_modelData; }

	private:

		Struct::ModelData m_modelData = {};
	};
}