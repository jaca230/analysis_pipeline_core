#include "analysis_pipeline/core/stages/histograms/th1_builder_stage.h"
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
    try {
        // Check that the input product exists before attempting checkout
        if (!getDataProductManager()->hasProduct(inputProductName_)) {
            spdlog::error("[{}] Input product '{}' not found", Name(), inputProductName_);
            return;
        }

        // Checkout input product for reading
        auto inputHandle = getDataProductManager()->checkoutRead(inputProductName_);
        if (!inputHandle.get()) {
            spdlog::error("[{}] Failed to lock input product '{}'", Name(), inputProductName_);
            return;
        }

        // Extract member pointer and type
        auto [memberPtr, memberType] = inputHandle->getMemberPointerAndType(valueKey_);
        if (!memberPtr) {
            spdlog::error("[{}] Member '{}' not found in product '{}'", Name(), valueKey_, inputProductName_);
            return;
        }

        // Attempt to convert the value into a double
        double valueToFill = 0.0;
        bool canFill = false;

        if (memberType == "double" || memberType == "Double_t") {
            valueToFill = *static_cast<const double*>(memberPtr);
            canFill = true;
        } else if (memberType == "float" || memberType == "Float_t") {
            valueToFill = static_cast<double>(*static_cast<const float*>(memberPtr));
            canFill = true;
        } else if (memberType == "int" || memberType == "Int_t") {
            valueToFill = static_cast<double>(*static_cast<const int*>(memberPtr));
            canFill = true;
        } else if (memberType == "short" || memberType == "Short_t") {
            valueToFill = static_cast<double>(*static_cast<const short*>(memberPtr));
            canFill = true;
        } else {
            spdlog::error("[{}] Unsupported member type '{}'", Name(), memberType);
            return;
        }

        if (!canFill) return;

        // Create the histogram if it doesn't exist
        if (!getDataProductManager()->hasProduct(histogramName_)) {
            auto newHist = std::make_unique<TH1D>(histogramName_.c_str(), title_.c_str(), bins_, min_, max_);
            auto newProduct = std::make_unique<PipelineDataProduct>();
            newProduct->setName(histogramName_);
            newProduct->setObject(std::move(newHist));
            newProduct->addTag("histogram");
            newProduct->addTag("built_by_th1_builder");
            getDataProductManager()->addOrUpdate(histogramName_, std::move(newProduct));
        }

        // Checkout histogram product for writing
        auto histHandle = getDataProductManager()->checkoutWrite(histogramName_);
        if (!histHandle.get()) {
            spdlog::error("[{}] Failed to lock histogram product '{}'", Name(), histogramName_);
            return;
        }

        auto* hist = dynamic_cast<TH1*>(histHandle.get()->getObject());
        if (!hist) {
            spdlog::error("[{}] Object named '{}' exists but is not a TH1", Name(), histogramName_);
            return;
        }

        // Fill the histogram
        hist->Fill(valueToFill);

    } catch (const std::exception& e) {
        spdlog::error("[{}] Exception in Process: {}", Name(), e.what());
    }
}



