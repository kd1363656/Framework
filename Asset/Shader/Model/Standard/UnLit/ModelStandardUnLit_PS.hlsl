#include "../ModelStandard.hlsli"
#include "ModelStandardUnLit.hlsli"

float4 main(const MSOutputUnLit a_input) : SV_Target0
{
    return FetchModelBaseColor(a_input.uv);
}