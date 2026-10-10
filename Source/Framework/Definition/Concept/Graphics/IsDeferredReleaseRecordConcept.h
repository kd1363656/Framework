#pragma once

namespace FWK::Graphics
{
    class ResourceReleaseContext;
}

namespace FWK::Concept
{
    template <typename Type>
    concept IsDeferredReleaseRecordConcept = requires(Type& a_record, const UINT64& a_retiredFenceValue, Graphics::ResourceReleaseContext& a_resourceReleaseContext)
    {
        { a_record.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext) } -> std::same_as<bool>;
    };
}