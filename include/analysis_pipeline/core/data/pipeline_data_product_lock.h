#pragma once

#include <shared_mutex>
#include <mutex>
#include <variant>
#include <memory>

class PipelineDataProduct;

class PipelineDataProductLock {
public:
    PipelineDataProductLock(PipelineDataProductLock&&) noexcept = default;
    PipelineDataProductLock& operator=(PipelineDataProductLock&&) noexcept = default;
    PipelineDataProductLock(const PipelineDataProductLock&) = delete;
    PipelineDataProductLock& operator=(const PipelineDataProductLock&) = delete;

    PipelineDataProduct* operator->() { return product_; }
    PipelineDataProduct& operator*() { return *product_; }
    PipelineDataProduct* get() { return product_; }
    bool valid() const { return product_ != nullptr; }

    ~PipelineDataProductLock() = default;

private:
    friend class PipelineDataProductManager;

    PipelineDataProductLock(PipelineDataProduct* prod,
        std::variant<std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>>&& lock);

    PipelineDataProduct* product_ = nullptr;
    std::variant<std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>> lock_;
};
