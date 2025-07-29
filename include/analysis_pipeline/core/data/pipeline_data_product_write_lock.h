#pragma once

#include <shared_mutex>
#include <mutex>
#include "analysis_pipeline/core/data/pipeline_data_product_lock.h"

class PipelineDataProduct;

class PipelineDataProductWriteLock : public PipelineDataProductLock {

public:
    PipelineDataProductWriteLock() noexcept = default;
    PipelineDataProduct* operator->() noexcept;
    PipelineDataProduct& operator*() noexcept;
    PipelineDataProduct* get() noexcept;

private:
    friend class PipelineDataProductManager;
    PipelineDataProductWriteLock(PipelineDataProduct* prod, std::unique_lock<std::shared_mutex>&& lock);

    std::unique_lock<std::shared_mutex> lock_;
};
