#ifndef ANALYSIS_PIPELINE_STAGES_CUSTOMMIDASUNPACKERSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_CUSTOMMIDASUNPACKERSTAGE_H

#include "base_midas_unpacker_stage.h"

#include <string>
#include <memory>
#include <vector>
#include <map>

class TTree;

namespace dataProducts {
  class DataProduct;
  using DataProductPtrCollection = std::vector<std::shared_ptr<DataProduct>>;
}

namespace unpackers {
  class EventUnpacker;
}

class CustomMidasUnpackerStage : public BaseMidasUnpackerStage {
public:
  CustomMidasUnpackerStage();
  ~CustomMidasUnpackerStage() override;

  std::string Name() const override { return "CustomMidasUnpackerStage"; }

protected:
  void OnInit() override;
  void ProcessMidasEvent(const TMEvent& event) override;

private:
  std::string unpacker_type_;  // Provided in config JSON
  std::unique_ptr<unpackers::EventUnpacker> unpacker_;
  std::map<std::string, std::shared_ptr<dataProducts::DataProductPtrCollection>> collections_;

  ClassDef(CustomMidasUnpackerStage, 1)
};

#endif // ANALYSIS_PIPELINE_STAGES_CUSTOMMIDASUNPACKERSTAGE_H
