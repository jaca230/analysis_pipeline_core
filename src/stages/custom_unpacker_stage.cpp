#include "custom_midas_unpacker_stage.h"

#include "common/unpacking/EventUnpacker.hh"
#include "common/unpacking/CollectionsHolder.hh"
#include "common/data_products/DataProduct.hh"

#include <TClass.h>
#include <TTree.h>
#include <TObject.h>

#include <iostream>
#include <stdexcept>

CustomMidasUnpackerStage::CustomMidasUnpackerStage() = default;
CustomMidasUnpackerStage::~CustomMidasUnpackerStage() = default;

void CustomMidasUnpackerStage::OnInit() {
    if (!parameters_.contains("unpacker_type")) {
        throw std::runtime_error("CustomMidasUnpackerStage::OnInit: missing 'unpacker_type' in config");
    }

    unpacker_type_ = parameters_.at("unpacker_type").get<std::string>();

    TClass* cls = TClass::GetClass(unpacker_type_.c_str());
    if (!cls) {
        throw std::runtime_error("CustomMidasUnpackerStage::OnInit: Class " + unpacker_type_ + " not found in ROOT dictionary");
    }

    TObject* obj = cls->New();
    if (!obj) {
        throw std::runtime_error("CustomMidasUnpackerStage::OnInit: Could not instantiate class " + unpacker_type_);
    }

    unpacker_.reset(dynamic_cast<unpackers::EventUnpacker*>(obj));
    if (!unpacker_) {
        delete obj;
        throw std::runtime_error("CustomMidasUnpackerStage::OnInit: Object is not an EventUnpacker");
    }
}

void CustomMidasUnpackerStage::ProcessMidasEvent(const TMEvent& event) {
    if (!unpacker_) {
        throw std::runtime_error("CustomMidasUnpackerStage::ProcessMidasEvent: unpacker not initialized");
    }

    int status = unpacker_->UnpackEvent(const_cast<TMEvent*>(&event));
    if (status != 0) {
        std::cerr << "CustomMidasUnpackerStage: Unpacking failed with status " << status << std::endl;
        return;
    }

    collections_ = unpacker_->GetCollections();

    SafeTreeAccess([this](TTree* tree) {
        for (const auto& [label, collection] : collections_) {
            for (const auto& obj : *collection) {
                tree->SetBranchAddress(label.c_str(), nullptr); // Assume branch already exists
                tree->Fill();
            }
        }
    });
}
