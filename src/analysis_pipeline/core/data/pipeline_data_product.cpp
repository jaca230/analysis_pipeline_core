#include "analysis_pipeline/core/data/pipeline_data_product.h"

#include <TBufferJSON.h>
#include <TClass.h>
#include <TDataMember.h>
#include <TCollection.h>
#include <TList.h>
#include <TString.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

// Object setters
void PipelineDataProduct::setObject(std::unique_ptr<TObject> obj) {
    if (!obj) {
        spdlog::warn("PipelineDataProduct::setObject called with null unique_ptr");
        return;
    }
    object_ = std::shared_ptr<TObject>(std::move(obj));
}

void PipelineDataProduct::setSharedObject(std::shared_ptr<TObject> obj) {
    if (!obj) {
        spdlog::warn("PipelineDataProduct::setSharedObject called with null shared_ptr");
        return;
    }
    object_ = std::move(obj);
}

// Object accessor
TObject* PipelineDataProduct::getObject() const {
    return object_ ? object_.get() : nullptr;
}

std::shared_ptr<TObject> PipelineDataProduct::getSharedObject() const {
    return object_;
}

// Name
const std::string& PipelineDataProduct::getName() const {
    return name_;
}

void PipelineDataProduct::setName(const std::string& name) {
    name_ = name;
}

// Class name
std::string PipelineDataProduct::getClassName() const {
    if (TObject* obj = getObject()) {
        return obj->IsA()->GetName();
    }
    return "";
}

// Member lookup
std::pair<void*, std::string> PipelineDataProduct::getMemberPointerAndType(const std::string& memberName) const {
    TObject* obj = getObject();
    if (!obj) return {nullptr, ""};

    TClass* cls = obj->IsA();
    if (!cls) return {nullptr, ""};

    TDataMember* dm = cls->GetDataMember(memberName.c_str());
    if (!dm) {
        spdlog::warn("Member '{}' not found in class '{}'", memberName, cls->GetName());
        return {nullptr, ""};
    }

    void* ptr = reinterpret_cast<char*>(obj) + dm->GetOffset();
    return {ptr, dm->GetFullTypeName()};
}

// All members
std::map<std::string, std::pair<void*, std::string>> PipelineDataProduct::getAllMembers() const {
    std::map<std::string, std::pair<void*, std::string>> members;
    TObject* obj = getObject();
    if (!obj) return members;

    TClass* cls = obj->IsA();
    if (!cls) return members;

    TIter next(cls->GetListOfDataMembers());
    TDataMember* dm = nullptr;
    while ((dm = dynamic_cast<TDataMember*>(next()))) {
        void* ptr = reinterpret_cast<char*>(obj) + dm->GetOffset();
        members[dm->GetName()] = {ptr, dm->GetFullTypeName()};
    }

    return members;
}

// Serialization
nlohmann::json PipelineDataProduct::serializeToJson() const {
    TObject* obj = getObject();
    if (!obj) return {};

    try {
        TString jsonStr = TBufferJSON::ConvertToJSON(obj);
        return nlohmann::json::parse(jsonStr.Data());
    } catch (const std::exception& e) {
        spdlog::error("Failed to serialize PipelineDataProduct '{}': {}", name_, e.what());
        return {};
    }
}

// Tags
void PipelineDataProduct::addTag(const std::string& tag) {
    tags_.insert(tag);
}

void PipelineDataProduct::removeTag(const std::string& tag) {
    tags_.erase(tag);
}

bool PipelineDataProduct::hasTag(const std::string& tag) const {
    return tags_.find(tag) != tags_.end();
}

const std::unordered_set<std::string>& PipelineDataProduct::getTags() const {
    return tags_;
}
