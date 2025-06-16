#include "base_stage.h"

BaseStage::BaseStage() = default;
BaseStage::~BaseStage() = default;

void BaseStage::Init(const nlohmann::json& parameters, TTree* tree, std::mutex* tree_mutex) {
    parameters_ = parameters;
    tree_ = tree;
    tree_mutex_ = tree_mutex;
    OnInit();
}
