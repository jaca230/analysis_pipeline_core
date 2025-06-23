#include "stages/unpacking/custom_midas_unpacker_stage.h"

#include <TClass.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

CustomMidasUnpackerStage::CustomMidasUnpackerStage() = default;

CustomMidasUnpackerStage::~CustomMidasUnpackerStage() = default;

void CustomMidasUnpackerStage::OnInit() {
    try {
        unpackerClassName_ = parameters_.at("unpacker_class").get<std::string>();
    } catch (const std::exception& e) {
        spdlog::error("[CustomMidasUnpackerStage] Missing or invalid 'unpacker_class' config: {}", e.what());
        throw;
    }

    TClass* cls = TClass::GetClass(unpackerClassName_.c_str());
    if (!cls) {
        spdlog::error("[CustomMidasUnpackerStage] Unknown unpacker class '{}'", unpackerClassName_);
        throw std::runtime_error("Unknown unpacker class: " + unpackerClassName_);
    }

    TObject* obj = static_cast<TObject*>(cls->New());
    if (!obj) {
        spdlog::error("[CustomMidasUnpackerStage] Failed to instantiate unpacker of class '{}'", unpackerClassName_);
        throw std::runtime_error("Failed to instantiate: " + unpackerClassName_);
    }

    unpacker_.reset(dynamic_cast<unpackers::EventUnpacker*>(obj));
    if (!unpacker_) {
        delete obj;
        spdlog::error("[CustomMidasUnpackerStage] Unpacker '{}' is not derived from EventUnpacker", unpackerClassName_);
        throw std::runtime_error("Unpacker is not derived from EventUnpacker");
    }

    spdlog::debug("[CustomMidasUnpackerStage] Instantiated unpacker of type '{}'", unpackerClassName_);
}

void CustomMidasUnpackerStage::ProcessMidasEvent(const TMEvent& event) {
    if (!unpacker_) {
        spdlog::error("[CustomMidasUnpackerStage] No unpacker instantiated; skipping event");
        return;
    }

    TMEvent& mutable_event = const_cast<TMEvent&>(event);
    mutable_event.FindAllBanks();

    unpacker_->UnpackEvent(&mutable_event);

    const auto& collections = unpacker_->GetCollections();

    std::unordered_map<std::string, std::unique_ptr<PipelineDataProduct>> newProducts;

    for (const auto& [label, vecPtr] : collections) {
        if (!vecPtr || vecPtr->empty()) continue;

        auto list = std::make_unique<TList>();
        list->SetOwner(kTRUE);

        for (const auto& shared_dp : *vecPtr) {
            if (!shared_dp) continue;
            TObject* clone = shared_dp->Clone();
            if (!clone) continue;
            list->Add(clone);
        }

        if (list->IsEmpty()) continue;

        auto pdp = std::make_unique<PipelineDataProduct>();
        pdp->setName(label);
        pdp->setObject(std::move(list));

        newProducts[label] = std::move(pdp);
    }

    getDataProductManager()->withProducts([&](auto& products) {
        for (auto& [name, pdp] : newProducts) {
            products[name] = std::move(pdp);
        }
    });

    spdlog::debug("[CustomMidasUnpackerStage] Registered {} PipelineDataProducts from unpacker '{}'",
                  newProducts.size(), unpackerClassName_);
}

std::string CustomMidasUnpackerStage::Name() const {
    return "CustomMidasUnpackerStage";
}
