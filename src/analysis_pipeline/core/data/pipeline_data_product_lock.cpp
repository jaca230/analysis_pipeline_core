#include "analysis_pipeline/core/data/pipeline_data_product_lock.h"


bool PipelineDataProductLock::valid() const noexcept {
    return product_ != nullptr;
}

PipelineDataProductLock::operator bool() const noexcept {
    return valid();
}

PipelineDataProductLock::PipelineDataProductLock(PipelineDataProduct* prod)
    : product_(prod) {}
