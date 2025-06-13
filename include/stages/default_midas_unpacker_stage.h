#ifndef DEFAULT_MIDAS_UNPACKER_STAGE_H
#define DEFAULT_MIDAS_UNPACKER_STAGE_H

#include "base_midas_unpacker_stage.h"
#include <nlohmann/json.hpp>

class DefaultMidasUnpackerStage : public BaseMidasUnpackerStage {
public:
    DefaultMidasUnpackerStage();
    ~DefaultMidasUnpackerStage() override;

    void ProcessMidasEvent(const TMEvent& event) override;

private:
    nlohmann::json decodeBankData(const TMBank& bank, const TMEvent& event) const;
    std::string toHexString(const char* data, size_t size) const;
};

#endif // DEFAULT_MIDAS_UNPACKER_STAGE_H
