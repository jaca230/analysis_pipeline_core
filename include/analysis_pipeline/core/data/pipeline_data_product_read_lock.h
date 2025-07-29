#pragma once

#include <shared_mutex>
#include <mutex>
#include "analysis_pipeline/core/data/pipeline_data_product_lock.h"

class PipelineDataProduct;

class PipelineDataProductReadLock : public PipelineDataProductLock {

public:
    PipelineDataProductReadLock() noexcept = default;
    const PipelineDataProduct* operator->() const noexcept;
    const PipelineDataProduct& operator*() const noexcept;
    const PipelineDataProduct* get() const noexcept;

private:
    friend class PipelineDataProductManager;
    PipelineDataProductReadLock(PipelineDataProduct* prod, std::shared_lock<std::shared_mutex>&& lock);

    std::shared_lock<std::shared_mutex> lock_;
};
