#include "stages/data_management/clear_products_stage.h"
#include <spdlog/spdlog.h>

ClassImp(ClearProductsStage) 

void ClearProductsStage::OnInit() {
    if (!parameters_.contains("products") || !parameters_["products"].is_array()) {
        throw std::runtime_error("ClearProductsStage requires a 'products' array in configuration");
    }

    productsToClear_.clear();
    for (const auto& name : parameters_["products"]) {
        productsToClear_.emplace_back(name.get<std::string>());
    }

    spdlog::debug("[{}] Initialized with {} products to clear",
                  Name(), productsToClear_.size());
}

void ClearProductsStage::Process() {
    getDataProductManager()->withProducts([&](auto& products) {
        for (const auto& name : productsToClear_) {
            if (products.erase(name) > 0) {
                spdlog::debug("[{}] Cleared product '{}'", Name(), name);
            } else {
                spdlog::debug("[{}] Product '{}' not found", Name(), name);
            }
        }
    });
}
