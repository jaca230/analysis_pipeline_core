#ifndef ANALYSISPIPELINE_PIPELINE_DATA_PRODUCT_H
#define ANALYSISPIPELINE_PIPELINE_DATA_PRODUCT_H

#include <TObject.h>
#include <TClass.h>
#include <TDataMember.h>
#include <TBufferJSON.h>
#include <TCollection.h>
#include <TList.h>

#include <memory>
#include <string>
#include <map>
#include <nlohmann/json.hpp>

class PipelineDataProduct {
public:
    PipelineDataProduct() = default; // default ctor required by ROOT and STL containers

    // Getters
    const std::string& getName() const { return name_; }
    std::string getClassName() const;
    TObject* getObject() const { return object_.get(); }

    // Setters
    void setObject(std::unique_ptr<TObject> obj);
    void setName(const std::string& name);

    // Member access
    std::pair<void*, std::string> getMemberPointerAndType(const std::string& memberName) const;
    std::map<std::string, std::pair<void*, std::string>> getAllMembers() const;

    // Serialization
    nlohmann::json serializeToJson() const;

private:
    std::unique_ptr<TObject> object_;
    std::string name_;
};

#endif // ANALYSISPIPELINE_PIPELINE_DATA_PRODUCT_H
