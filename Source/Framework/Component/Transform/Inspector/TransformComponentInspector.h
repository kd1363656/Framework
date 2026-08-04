#pragma once

namespace FWK
{
	class TransformComponent;
}

namespace FWK
{
	class TransformComponentInspector final
	{
	public:

		 TransformComponentInspector() = default;
		~TransformComponentInspector() = default;

		void EditInspector(TransformComponent& a_transformComponent);

	private:

		static constexpr std::string_view k_matrixStrategySelectorLabel = "行列計算方法セレクター";
		static constexpr std::string_view k_transformPositionLabel      = "位置";
		static constexpr std::string_view k_transformRotationLabel      = "回転";
		static constexpr std::string_view k_transformScaleLabel         = "拡大";
	};
}