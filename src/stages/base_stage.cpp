#include "stages/base_stage.h"

BaseStage::BaseStage() = default;
BaseStage::~BaseStage() = default;

void BaseStage::Init(const nlohmann::json& parameters, TTree* tree, std::mutex* tree_mutex) {
    parameters_ = parameters;
    tree_ = tree;
    tree_mutex_ = tree_mutex;
    OnInit();
}

template <typename Func>
void BaseStage::SafeTreeAccess(Func func) {
    if (!tree_ || !tree_mutex_) {
        throw std::runtime_error("BaseStage: Tree or mutex pointer not initialized");
    }
    std::lock_guard<std::mutex> lock(*tree_mutex_);
    func(tree_);
}

// Explicit template instantiation if needed (for common usage)
// template void BaseStage::SafeTreeAccess<>(std::function<void(TTree*)>);
