#ifndef ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H

#include "stages/base_stage.h"
#include "context/input_bundle.h"

class BaseInputStage : public BaseStage {
public:
    BaseInputStage() = default;
    ~BaseInputStage() override = default;

    // Receives externally injected input as InputBundle reference.
    virtual void SetInput(const InputBundle& input) = 0;

    ClassDefOverride(BaseInputStage, 2);
};

#endif // ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H
