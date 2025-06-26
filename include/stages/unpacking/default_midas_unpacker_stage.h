#ifndef DEFAULT_MIDAS_UNPACKER_STAGE_H
#define DEFAULT_MIDAS_UNPACKER_STAGE_H

#include "stages/unpacking/base_midas_unpacker_stage.h"
#include <nlohmann/json.hpp>

class DefaultMidasUnpackerStage : public BaseMidasUnpackerStage {
public:
    DefaultMidasUnpackerStage();
    ~DefaultMidasUnpackerStage() override;

    void ProcessMidasEvent(TMEvent& event) override;

    std::string Name() const override;

private:
    nlohmann::json decodeBankData(const TMBank& bank, TMEvent& event) const;
    std::string toHexString(const char* data, size_t size) const;
    std::string event_json_str_;

    ClassDefOverride(DefaultMidasUnpackerStage, 1);  // Use ClassDefOverride for ROOT compatibility
};

#endif // DEFAULT_MIDAS_UNPACKER_STAGE_H
