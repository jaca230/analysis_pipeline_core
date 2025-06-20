#ifndef ANALYSIS_PIPELINE_STAGES_RANDOM_DATA_GENERATOR_STAGE_H
#define ANALYSIS_PIPELINE_STAGES_RANDOM_DATA_GENERATOR_STAGE_H

#include "stages/base_stage.h"
#include <random>

class RandomDataGeneratorStage : public BaseStage {
public:
    RandomDataGeneratorStage();
    ~RandomDataGeneratorStage() override = default;

    void Process() override;
    std::string Name() const override { return "RandomDataGeneratorStage"; }

protected:
    void OnInit() override;

private:
    std::string productName_;
    double minValue_ = 0.0;
    double maxValue_ = 1.0;
    unsigned int seed_ = 0;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
};

#endif // ANALYSIS_PIPELINE_STAGES_RANDOM_DATA_GENERATOR_STAGE_H
