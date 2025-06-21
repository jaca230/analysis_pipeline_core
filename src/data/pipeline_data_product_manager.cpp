#include "data/pipeline_data_product_manager.h"
#include <spdlog/spdlog.h>

void PipelineDataProductManager::addOrUpdate(const std::string& name, std::unique_ptr<PipelineDataProduct> product) {
    if (!product) {
        spdlog::warn("[PipelineDataProductManager] Tried to add/update null product for '{}'", name);
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    product->setName(name);
    products_[name] = std::move(product);
}

std::vector<std::string> PipelineDataProductManager::getAllNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    names.reserve(products_.size());
    for (const auto& [name, _] : products_) {
        names.push_back(name);
    }
    return names;
}

void PipelineDataProductManager::remove(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    products_.erase(name);
}

// Thread-safe clear all data products
void PipelineDataProductManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    products_.clear();
}

nlohmann::json PipelineDataProductManager::serializeAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::object();

    for (const auto& [name, product] : products_) {
        if (product) {
            try {
                result[name] = product->serializeToJson();
            } catch (const std::exception& e) {
                spdlog::error("[PipelineDataProductManager] Serialization failed for '{}': {}", name, e.what());
            }
        }
    }
    return result;
}

void PipelineDataProductManager::withProduct(
    const std::string& name,
    const std::function<void(std::unique_ptr<PipelineDataProduct>&)>& fn
) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& productPtr = products_[name]; // May be nullptr
    fn(productPtr);
    if (productPtr) {
        productPtr->setName(name);
    }
}

void PipelineDataProductManager::withProducts(
    const std::function<void(
        std::unordered_map<std::string, std::unique_ptr<PipelineDataProduct>>&
    )>& fn
) {
    std::lock_guard<std::mutex> lock(mutex_);
    fn(products_);
}

void PipelineDataProductManager::forEachProductConst(
    const std::function<void(const std::string&, const PipelineDataProduct&)>& fn
) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [name, product] : products_) {
        if (product) {
            fn(name, *product);
        }
    }
}
