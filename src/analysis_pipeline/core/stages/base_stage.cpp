#include "analysis_pipeline/core/stages/base_stage.h"
#include <stdexcept>

BaseStage::BaseStage() = default;
BaseStage::~BaseStage() = default;

void BaseStage::Init(const nlohmann::json& parameters,
                     PipelineDataProductManager* dataProductManager) {
    parameters_ = parameters;
    dataProductManager_ = dataProductManager;
    OnInit();
}