#include "stages/histograms/th1_builder_stage.h"
#include <TParameter.h>
#include <spdlog/spdlog.h>

ClassImp(TH1BuilderStage)

void TH1BuilderStage::OnInit() {
    inputProductName_ = parameters_.value("input_product", "");
    histogramName_ = parameters_.value("product_name", "hist");
    valueKey_ = parameters_.value("value_key", "value");
    title_ = parameters_.value("title", histogramName_);
    bins_ = parameters_.value("bins", 100);
    min_ = parameters_.value("min", 0.0);
    max_ = parameters_.value("max", 1.0);

    if (inputProductName_.empty()) {
        throw std::runtime_error("TH1BuilderStage: input_product is required");
    }

    spdlog::debug("[{}] Configured to read from '{}', extract key '{}', and fill '{}'",
                 Name(), inputProductName_, valueKey_, histogramName_);
}

void TH1BuilderStage::Process() {
    getDataProductManager()->withProducts([&](auto& products) {
        // Find the input data product by name
        auto itInput = products.find(inputProductName_);
        if (itInput == products.end() || !itInput->second) {
            spdlog::error("[{}] Input product '{}' not found", Name(), inputProductName_);
            return;
        }

        PipelineDataProduct* inputProduct = itInput->second.get();

        // Extract member pointer and type
        auto [memberPtr, memberType] = inputProduct->getMemberPointerAndType(valueKey_);
        if (!memberPtr) {
            spdlog::error("[{}] Member '{}' not found in product '{}'", Name(), valueKey_, inputProductName_);
            return;
        }

        // We'll store the extracted value here if possible
        double valueToFill = 0.0;
        bool canFill = false;

        // Cast based on type string — handle supported types
        if (memberType == "double" || memberType == "Double_t") {
            valueToFill = *static_cast<const double*>(memberPtr);
            canFill = true;
        }
        else if (memberType == "float" || memberType == "Float_t") {
            valueToFill = static_cast<double>(*static_cast<const float*>(memberPtr));
            canFill = true;
        }
        else if (memberType == "int" || memberType == "Int_t") {
            valueToFill = static_cast<double>(*static_cast<const int*>(memberPtr));
            canFill = true;
        }
        else if (memberType == "short" || memberType == "Short_t") {
            valueToFill = static_cast<double>(*static_cast<const short*>(memberPtr));
            canFill = true;
        }
        else {
            spdlog::error("[{}] Unsupported member type '{}'", Name(), memberType);
            return;
        }

        // Find or create the histogram product
        auto itHist = products.find(histogramName_);

        TH1* hist = nullptr;

        if (itHist != products.end() && itHist->second) {
            // Existing histogram product found
            TObject* obj = itHist->second->getObject();
            hist = dynamic_cast<TH1*>(obj);
            if (!hist) {
                spdlog::error("[{}] Object named '{}' exists but is not a TH1", Name(), histogramName_);
                return;
            }
        }
        else {
            // Create new histogram of type TH1D (for example)
            auto newHist = std::make_unique<TH1D>(histogramName_.c_str(), title_.c_str(), bins_, min_, max_);

            // Create new PipelineDataProduct wrapping the histogram
            auto newProduct = std::make_unique<PipelineDataProduct>();
            newProduct->setName(histogramName_);
            newProduct->setObject(std::move(newHist));

            hist = dynamic_cast<TH1*>(newProduct->getObject());
            products[histogramName_] = std::move(newProduct);
        }

        // Fill the histogram
        hist->Fill(valueToFill);
    });
}

