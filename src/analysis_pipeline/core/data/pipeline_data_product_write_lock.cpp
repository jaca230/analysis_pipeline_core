#include "analysis_pipeline/core/data/pipeline_data_product_write_lock.h"

PipelineDataProductWriteLock::PipelineDataProductWriteLock(
    PipelineDataProduct* prod,
    std::unique_lock<std::shared_mutex>&& lock)
    : PipelineDataProductLock(prod), lock_(std::move(lock)) {}

PipelineDataProduct* PipelineDataProductWriteLock::operator->() noexcept {
    return product_;
}

PipelineDataProduct& PipelineDataProductWriteLock::operator*() noexcept {
    return *product_;
}

PipelineDataProduct* PipelineDataProductWriteLock::get() noexcept {
    return product_;
}
