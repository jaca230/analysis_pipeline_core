#include "analysis_pipeline/core/data/pipeline_data_product_read_lock.h"

PipelineDataProductReadLock::PipelineDataProductReadLock(
    PipelineDataProduct* prod,
    std::shared_lock<std::shared_mutex>&& lock)
    : PipelineDataProductLock(prod), lock_(std::move(lock)) {}

const PipelineDataProduct* PipelineDataProductReadLock::operator->() const noexcept {
    return product_;
}

const PipelineDataProduct& PipelineDataProductReadLock::operator*() const noexcept {
    return *product_;
}

const PipelineDataProduct* PipelineDataProductReadLock::get() const noexcept {
    return product_;
}
