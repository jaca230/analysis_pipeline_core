#ifndef ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H

#include "stages/base_stage.h"
#include <vector>
#include <string>

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
    
    ClassDefOverride(ClearProductsStage, 1);
};

#endif // ANALYSIS_PIPELINE_STAGES_CLEARPRODUCTSSTAGE_H
