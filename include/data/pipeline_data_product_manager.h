// data/pipeline_data_product_manager.h
#ifndef PIPELINE_DATA_PRODUCT_MANAGER_H
#define PIPELINE_DATA_PRODUCT_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include "pipeline_data_product.h"

class PipelineDataProductManager {
public:
    PipelineDataProductManager() = default;

    // Thread-safe add or update a product
    void addOrUpdate(const std::string& name, std::unique_ptr<PipelineDataProduct> product);

    // Thread-safe get a product pointer, nullptr if not found
    PipelineDataProduct* get(const std::string& name);

    // Thread-safe remove a product by name
    void remove(const std::string& name);

    // Thread-safe serialize all products to JSON
    nlohmann::json serializeAll() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<PipelineDataProduct>> products_;
};

#endif // PIPELINE_DATA_PRODUCT_MANAGER_H
