#include "data/pipeline_data_product.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

std::string PipelineDataProduct::getClassName() const {
    if (!object_) return "";
    return object_->IsA()->GetName();
}

void PipelineDataProduct::setObject(std::unique_ptr<TObject> obj) {
    if (!obj) {
        throw std::invalid_argument("setObject received null TObject");
    }
    object_ = std::move(obj);
    if (object_->GetName()) {
        name_ = object_->GetName();
    }
}

void PipelineDataProduct::setName(const std::string& name) {
    name_ = name;
}

std::pair<void*, std::string> PipelineDataProduct::getMemberPointerAndType(const std::string& memberName) const {
    if (!object_) return {nullptr, ""};

    TClass* cls = object_->IsA();
    if (!cls) return {nullptr, ""};

    TDataMember* dm = cls->GetDataMember(memberName.c_str());
    if (!dm) {
        spdlog::warn("[PipelineDataProduct] Member '{}' not found in class '{}'", memberName, cls->GetName());
        return {nullptr, ""};
    }

    void* ptr = (char*)object_.get() + dm->GetOffset();
    return {ptr, dm->GetFullTypeName()};
}

std::map<std::string, std::pair<void*, std::string>> PipelineDataProduct::getAllMembers() const {
    std::map<std::string, std::pair<void*, std::string>> members;

    if (!object_) return members;

    TClass* cls = object_->IsA();
    if (!cls) return members;

    const TCollection* membersList = cls->GetListOfDataMembers();
    TIter next(membersList);
    TDataMember* dm;
    while ((dm = (TDataMember*)next())) {
        void* ptr = (char*)object_.get() + dm->GetOffset();
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
        spdlog::error("[PipelineDataProduct] JSON serialization failed: {}", e.what());
        return nlohmann::json{};
    }
}
