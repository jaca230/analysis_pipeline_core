#ifndef ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASESTAGE_H

#include <TObject.h>
#include <TTree.h>
#include <string>
#include <mutex>
#include <nlohmann/json.hpp>

class BaseStage : public TObject {
public:
    BaseStage();
    virtual ~BaseStage();

    // Initialize stage parameters, tree pointer, and mutex pointer.
    void Init(const nlohmann::json& parameters, TTree* tree, std::mutex* tree_mutex);

    // Thread-safe wrapper to be implemented by derived classes
    virtual void Process() = 0;
    virtual std::string Name() const = 0;

protected:
    virtual void OnInit() {}  // Hook for derived classes to process parameters after Init

    // Helper to run any tree modification code safely with mutex locked
    template <typename Func>
    void SafeTreeAccess(Func func);

    nlohmann::json parameters_;

    TTree* tree_ = nullptr;         // Raw pointer, externally owned
    std::mutex* tree_mutex_ = nullptr; // Raw pointer, externally owned

    ClassDef(BaseStage, 1)
};

#endif // ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
