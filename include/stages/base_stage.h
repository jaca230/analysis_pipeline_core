#ifndef ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASESTAGE_H

#include <TObject.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "data/pipeline_data_product_manager.h"  // include manager

class BaseStage : public TObject {
public:
    BaseStage();
    virtual ~BaseStage();

    // Initialize stage parameters and pointer to shared data product manager
    void Init(const nlohmann::json& parameters,
              PipelineDataProductManager* dataProductManager);

    virtual void Process() = 0;
    virtual std::string Name() const = 0;

protected:
    virtual void OnInit() {}

    // Instead of direct map access, expose manager pointer to derived classes if needed
    PipelineDataProductManager* getDataProductManager() const { return dataProductManager_; }

    nlohmann::json parameters_;

private:
    // Pointer to shared manager (owned by Pipeline)
    PipelineDataProductManager* dataProductManager_ = nullptr;

    ClassDef(BaseStage, 2)  // Increment version due to interface change
};

#endif // ANALYSIS_PIPELINE_STAGES_BASESTAGE_H