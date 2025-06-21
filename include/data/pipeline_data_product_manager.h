#ifndef PIPELINE_DATA_PRODUCT_MANAGER_H
#define PIPELINE_DATA_PRODUCT_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <functional>
#include "pipeline_data_product.h"

class PipelineDataProductManager {
public:
    PipelineDataProductManager() = default;

    // Thread-safe add or update a product
    void addOrUpdate(const std::string& name, std::unique_ptr<PipelineDataProduct> product);

    // Thread-safe get list of all product names
    std::vector<std::string> getAllNames() const;

    // Thread-safe remove a product by name
    void remove(const std::string& name);

    // Thread-safe clear all data products
    void clear();

    // Thread-safe serialize all products to JSON
    nlohmann::json serializeAll() const;

    // Thread-safe access to a single product (read/write)
    void withProduct(const std::string& name,
                    const std::function<void(std::unique_ptr<PipelineDataProduct>&)>& fn);

    // Thread-safe access to all products (read/write)
    void withProducts(const std::function<void(
        std::unordered_map<std::string, std::unique_ptr<PipelineDataProduct>>&
    )>& fn);

    // Optional: thread-safe read-only access to all products
    void forEachProductConst(const std::function<void(const std::string&, const PipelineDataProduct&)>& fn) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<PipelineDataProduct>> products_;
};

#endif // PIPELINE_DATA_PRODUCT_MANAGER_H
