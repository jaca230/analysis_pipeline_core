#ifndef ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H

#include "stages/base_stage.h"
#include <any>

class BaseInputStage : public BaseStage {
public:
    BaseInputStage() = default;
    ~BaseInputStage() override = default;

    // Receives externally injected input (event, frame, chunk, etc.)
    virtual void SetInput(std::any input) = 0;

    ClassDefOverride(BaseInputStage, 1);
};

#endif // ANALYSIS_PIPELINE_STAGES_BASEINPUTSTAGE_H
