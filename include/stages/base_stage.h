#ifndef ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASESTAGE_H

#include <TObject.h>
#include <TTree.h>
#include <string>
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>

class BaseStage : public TObject {
public:
    BaseStage();
    virtual ~BaseStage();

    void Init(const nlohmann::json& parameters, TTree* tree, std::mutex* tree_mutex);
    void SetTree(TTree* tree, std::mutex* tree_mutex);  // <-- NEW METHOD

    virtual void Process() = 0;
    virtual std::string Name() const = 0;

protected:
    virtual void OnInit() {}

    template <typename Func>
    void SafeTreeAccess(Func func) {
        if (!tree_ || !tree_mutex_) {
            throw std::runtime_error("BaseStage: Tree or mutex pointer not initialized");
        }
        std::lock_guard<std::mutex> lock(*tree_mutex_);
        func(tree_);
    }

    nlohmann::json parameters_;
    TTree* tree_ = nullptr;
    std::mutex* tree_mutex_ = nullptr;

    ClassDef(BaseStage, 1)
};

#endif // ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
