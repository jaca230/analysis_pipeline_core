#pragma once

#include <memory>
#include <string>
#include <map>
#include <nlohmann/json.hpp>

#include <TObject.h>
#include <TClass.h>
#include <TDataMember.h>

/**
 * @class PipelineDataProduct
 * @brief Wraps a TObject and provides reflection and serialization utilities.
 */
class PipelineDataProduct {
public:
    PipelineDataProduct() = default;
    explicit PipelineDataProduct(std::unique_ptr<TObject> obj);

    // Accessors
    TObject* getObject() const;
    void setObject(std::unique_ptr<TObject> obj);

    const std::string& getName() const;
    void setName(const std::string& name);

    std::string getClassName() const;

    // ROOT Reflection Utilities
    std::pair<void*, std::string> getMemberPointerAndType(const std::string& memberName) const;
    std::map<std::string, std::pair<void*, std::string>> getAllMembers() const;

    // JSON Serialization
    nlohmann::json serializeToJson() const;

    // Tag Management
    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    const std::unordered_set<std::string>& getTags() const;

private:
    std::unique_ptr<TObject> object_;
    std::string name_;
    std::unordered_set<std::string> tags_;
};
