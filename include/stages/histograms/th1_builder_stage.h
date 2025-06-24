#ifndef ANALYSIS_PIPELINE_STAGES_TH1_BUILDER_STAGE_H
#define ANALYSIS_PIPELINE_STAGES_TH1_BUILDER_STAGE_H

#include "stages/base_stage.h"
#include <string>
#include <TH1D.h>

class TH1BuilderStage : public BaseStage {
public:
    TH1BuilderStage() = default;
    ~TH1BuilderStage() override = default;

    void Process() override;
    std::string Name() const override { return "TH1BuilderStage"; }

protected:
    void OnInit() override;

private:
    std::string inputProductName_;
    std::string histogramName_;
    std::string valueKey_;
    std::string title_;
    int bins_ = 100;
    double min_ = 0.0;
    double max_ = 1.0;

    ClassDefOverride(TH1BuilderStage, 1);
};

#endif // ANALYSIS_PIPELINE_STAGES_TH1_BUILDER_STAGE_H
