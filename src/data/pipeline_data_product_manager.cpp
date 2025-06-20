// data/pipeline_data_product_manager.cpp
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

PipelineDataProduct* PipelineDataProductManager::get(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = products_.find(name);
    if (it != products_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void PipelineDataProductManager::remove(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    products_.erase(name);
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
