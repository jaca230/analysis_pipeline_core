#ifndef ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H

#include "analysis_pipeline/core/stages/base_stage.h"
#include <vector>
#include <string>
#include <unordered_set>

class ClearProductsStage : public BaseStage {
public:
    ClearProductsStage() = default;
    virtual ~ClearProductsStage() = default;

    void Process() override;
    std::string Name() const override { return "ClearProductsStage"; }

protected:
    void OnInit() override;

private:
    std::vector<std::string> productsToClear_;
    std::unordered_set<std::string> tagsToClear_;

    ClassDefOverride(ClearProductsStage, 2);
};

#endif // ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H
