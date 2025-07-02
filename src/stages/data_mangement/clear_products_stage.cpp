#include "stages/data_management/clear_products_stage.h"
#include "spdlog/spdlog.h"

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
    if (productsToClear_.empty()) return;

    getDataProductManager()->removeMultiple(productsToClear_);

    for (const auto& name : productsToClear_) {
        spdlog::debug("[{}] Removed product '{}'", Name(), name);
    }
}
