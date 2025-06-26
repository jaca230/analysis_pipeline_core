#ifndef ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H

#include "stages/base_stage.h"
#include "midas.h"
#include "midasio.h"

class BaseMidasUnpackerStage : public BaseStage {
public:
    BaseMidasUnpackerStage();
    ~BaseMidasUnpackerStage() override;

    // TMEvent must be externally provided to this stage
    void SetCurrentEvent(TMEvent& event);

    // Core processing logic must be implemented by concrete unpackers
    void Process() final override;

protected:
    TMEvent* current_event_ = nullptr;

    // Derived classes implement this
    virtual void ProcessMidasEvent(TMEvent& event) = 0;

    ClassDef(BaseMidasUnpackerStage, 1)
};

#endif // ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H
