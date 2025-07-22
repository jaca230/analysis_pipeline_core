#pragma once

#include <shared_mutex>
#include <mutex>
#include <variant>
#include <memory>

class PipelineDataProduct;

class PipelineDataProductLock {
public:
    PipelineDataProductLock() noexcept = default;

    PipelineDataProductLock(PipelineDataProductLock&&) noexcept = default;
    PipelineDataProductLock& operator=(PipelineDataProductLock&&) noexcept = default;

    PipelineDataProductLock(const PipelineDataProductLock&) = delete;
    PipelineDataProductLock& operator=(const PipelineDataProductLock&) = delete;

    PipelineDataProduct* operator->() noexcept { return product_; }
    const PipelineDataProduct* operator->() const noexcept { return product_; }

    PipelineDataProduct& operator*() noexcept { return *product_; }
    const PipelineDataProduct& operator*() const noexcept { return *product_; }

    PipelineDataProduct* get() noexcept { return product_; }
    const PipelineDataProduct* get() const noexcept { return product_; }

    bool valid() const noexcept { return product_ != nullptr; }
    explicit operator bool() const noexcept { return valid(); }

    ~PipelineDataProductLock() = default;

private:
    friend class PipelineDataProductManager;

    PipelineDataProductLock(
        PipelineDataProduct* prod,
        std::variant<std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>>&& lock
    );

    PipelineDataProduct* product_ = nullptr;
    std::variant<std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>> lock_;
};
