#include "../ModelStandard.hlsli"

float4 main(const MSOutput a_input) : SV_Target0
{
    return FetchModelBaseColor(a_input.uv);

}