#include "data/pipeline_data_product.h"
#include <TBufferJSON.h>
#include <TCollection.h>
#include <TList.h>
#include "spdlog/spdlog.h"
#include <stdexcept>

PipelineDataProduct::PipelineDataProduct(std::unique_ptr<TObject> obj)
    : object_(std::move(obj)) {}

TObject* PipelineDataProduct::getObject() const {
    return object_.get();
}

void PipelineDataProduct::setObject(std::unique_ptr<TObject> obj) {
    if (!obj) {
        spdlog::warn("PipelineDataProduct::setObject called with null object");
        return;
    }
    object_ = std::move(obj);
}

const std::string& PipelineDataProduct::getName() const {
    return name_;
}

void PipelineDataProduct::setName(const std::string& name) {
    name_ = name;
}

std::string PipelineDataProduct::getClassName() const {
    if (!object_) return "";
    return object_->IsA()->GetName();
}

std::pair<void*, std::string> PipelineDataProduct::getMemberPointerAndType(const std::string& memberName) const {
    if (!object_) return {nullptr, ""};

    TClass* cls = object_->IsA();
    if (!cls) return {nullptr, ""};

    TDataMember* dm = cls->GetDataMember(memberName.c_str());
    if (!dm) {
        spdlog::warn("Member '{}' not found in class '{}'", memberName, cls->GetName());
        return {nullptr, ""};
    }

    void* ptr = reinterpret_cast<char*>(object_.get()) + dm->GetOffset();
    return {ptr, dm->GetFullTypeName()};
}

std::map<std::string, std::pair<void*, std::string>> PipelineDataProduct::getAllMembers() const {
    std::map<std::string, std::pair<void*, std::string>> members;

    if (!object_) return members;
    TClass* cls = object_->IsA();
    if (!cls) return members;

    const TCollection* list = cls->GetListOfDataMembers();
    TIter next(list);
    TDataMember* dm = nullptr;
    while ((dm = dynamic_cast<TDataMember*>(next()))) {
        void* ptr = reinterpret_cast<char*>(object_.get()) + dm->GetOffset();
        members[dm->GetName()] = {ptr, dm->GetFullTypeName()};
    }

    return members;
}

nlohmann::json PipelineDataProduct::serializeToJson() const {
    if (!object_) return nlohmann::json{};
    try {
        TString jsonStr = TBufferJSON::ConvertToJSON(object_.get());
        return nlohmann::json::parse(jsonStr.Data());
    } catch (const std::exception& e) {
        spdlog::error("Failed to serialize PipelineDataProduct '{}': {}", name_, e.what());
        return nlohmann::json{};
    }
}

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


