#pragma once

class PipelineDataProduct;

class PipelineDataProductLock {
public:
    PipelineDataProductLock() noexcept = default;
    PipelineDataProductLock(const PipelineDataProductLock&) = delete;
    PipelineDataProductLock& operator=(const PipelineDataProductLock&) = delete;
    PipelineDataProductLock(PipelineDataProductLock&&) noexcept = default;
    PipelineDataProductLock& operator=(PipelineDataProductLock&&) noexcept = default;
    virtual ~PipelineDataProductLock() = default;

    bool valid() const noexcept;
    explicit operator bool() const noexcept;

protected:
    explicit PipelineDataProductLock(PipelineDataProduct* prod);

    PipelineDataProduct* product_ = nullptr;
};
