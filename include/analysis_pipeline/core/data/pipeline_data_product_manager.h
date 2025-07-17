#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <variant>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "analysis_pipeline/core/data/pipeline_data_product.h"
#include "analysis_pipeline/core/data/pipeline_data_product_lock.h"

class PipelineDataProductManager {
public:
    PipelineDataProductManager() = default;

    void addOrUpdate(const std::string& name, std::unique_ptr<PipelineDataProduct> product);
    void addOrUpdateMultiple(std::vector<std::pair<std::string, std::unique_ptr<PipelineDataProduct>>>&& products);

    void remove(const std::string& name);
    void removeMultiple(const std::vector<std::string>& names);

    void clear();

    std::vector<std::string> getAllNames() const;

    bool hasProduct(const std::string& name) const;
    std::vector<bool> hasProducts(const std::vector<std::string>& names) const;
    std::vector<std::string> getExistingProducts(const std::vector<std::string>& names) const;

    PipelineDataProductLock checkoutRead(const std::string& name);
    PipelineDataProductLock checkoutWrite(const std::string& name);
    std::vector<PipelineDataProductLock> checkoutReadMultiple(const std::vector<std::string>& names);
    std::vector<PipelineDataProductLock> checkoutWriteMultiple(const std::vector<std::string>& names);

    nlohmann::json serializeAll() const;

    //tags
    std::unordered_set<std::string> getAllTags() const;

    // Single tag
    void removeByTag(const std::string& tag);
    void removeExcludingTag(const std::string& tag);
    std::vector<std::string> getNamesWithTag(const std::string& tag) const;

    // Multiple tags
    void removeByTags(const std::unordered_set<std::string>& tags);
    void removeExcludingTags(const std::unordered_set<std::string>& tags);
    std::vector<std::string> getNamesWithAnyTags(const std::unordered_set<std::string>& tags) const;
    std::vector<std::string> getNamesWithAllTags(const std::unordered_set<std::string>& tags) const;
    std::vector<std::string> getNamesWithExactTags(const std::unordered_set<std::string>& tags) const;
    std::vector<std::string> getNamesWithNoTags() const;

private:
    struct ProductEntry {
        std::unique_ptr<PipelineDataProduct> product;
        mutable std::shared_mutex mutex;
    };

    mutable std::shared_mutex managerMutex_;
    std::unordered_map<std::string, ProductEntry> products_;
};
