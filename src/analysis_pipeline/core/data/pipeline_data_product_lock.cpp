#include "analysis_pipeline/core/data/pipeline_data_product_lock.h"
#include "analysis_pipeline/core/data/pipeline_data_product.h"

PipelineDataProductLock::PipelineDataProductLock(
    PipelineDataProduct* prod,
    std::variant<std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>>&& lock
)
    : product_(prod), lock_(std::move(lock)) {}