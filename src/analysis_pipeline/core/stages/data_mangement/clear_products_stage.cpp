#include "analysis_pipeline/core/stages/data_management/clear_products_stage.h"
#include "spdlog/spdlog.h"

ClassImp(ClearProductsStage)

void ClearProductsStage::OnInit() {
    productsToClear_.clear();
    tagsToClear_.clear();

    if (parameters_.contains("products")) {
        if (!parameters_["products"].is_array()) {
            throw std::runtime_error("ClearProductsStage: 'products' must be an array");
        }
        for (const auto& name : parameters_["products"]) {
            productsToClear_.emplace_back(name.get<std::string>());
        }
    }

    if (parameters_.contains("tags")) {
        if (!parameters_["tags"].is_array()) {
            throw std::runtime_error("ClearProductsStage: 'tags' must be an array");
        }
        for (const auto& tag : parameters_["tags"]) {
            tagsToClear_.insert(tag.get<std::string>());
        }
    }

    spdlog::debug("[{}] Initialized with {} products and {} tags to clear",
                  Name(), productsToClear_.size(), tagsToClear_.size());
}

void ClearProductsStage::Process() {
    auto manager = getDataProductManager();
    std::unordered_set<std::string> toRemove;

    // explicit names
    toRemove.insert(productsToClear_.begin(), productsToClear_.end());

    // by tag
    for (const auto& tag : tagsToClear_) {
        auto taggedNames = manager->getNamesWithTag(tag);
        toRemove.insert(taggedNames.begin(), taggedNames.end());
    }

    if (toRemove.empty()) return;

    std::vector<std::string> removalList(toRemove.begin(), toRemove.end());
    manager->removeMultiple(removalList);

    for (const auto& name : removalList) {
        spdlog::debug("[{}] Removed product '{}'", Name(), name);
    }
}
