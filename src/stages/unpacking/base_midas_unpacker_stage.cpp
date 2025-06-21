#include "stages/unpacking/base_midas_unpacker_stage.h"
#include <stdexcept>

BaseMidasUnpackerStage::BaseMidasUnpackerStage() = default;
BaseMidasUnpackerStage::~BaseMidasUnpackerStage() = default;

void BaseMidasUnpackerStage::SetCurrentEvent(const TMEvent& event) {
    current_event_ = &event;
}

void BaseMidasUnpackerStage::Process() {
    if (!current_event_) {
        throw std::runtime_error("BaseMidasUnpackerStage: current_event_ not set");
    }
    ProcessMidasEvent(*current_event_);
}
