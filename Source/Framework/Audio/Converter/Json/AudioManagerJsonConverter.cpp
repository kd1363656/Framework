#include "AudioManagerJsonConverter.h"

void FWK::Converter::AudioManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, AudioManager& a_audioManager) const
{
	if (a_rootJson.is_null()) { return; }

	const float l_masterVolume = a_rootJson.value(k_masterVolumeJsonKey, Constant::k_defaultVolume);

	a_audioManager.ApplyMasterVolume(l_masterVolume);
}

nlohmann::json FWK::Converter::AudioManagerJsonConverter::Serialize(const AudioManager& a_audioManager) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_masterVolumeJsonKey] = a_audioManager.GetVALMasterVolume();

	return l_rootJson;
}