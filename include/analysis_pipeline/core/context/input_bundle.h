#ifndef ANALYSIS_PIPELINE_CONTEXT_INPUT_BUNDLE_H
#define ANALYSIS_PIPELINE_CONTEXT_INPUT_BUNDLE_H

#include <string>
#include <unordered_map>
#include <any>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <ostream>
#include <vector>

class InputBundle {
public:
    InputBundle() = default;
    ~InputBundle() = default;

    InputBundle(const InputBundle&) = delete;
    InputBundle& operator=(const InputBundle&) = delete;

    InputBundle(InputBundle&&) = default;
    InputBundle& operator=(InputBundle&&) = default;

    // Set any object by value (including shared_ptr<T>)
    template <typename T>
    void set(const std::string& key, T&& value) {
        data_[key] = std::forward<T>(value);
    }

    // Get stored object by type - fixed to handle references properly
    template <typename T>
    T get(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("InputBundle: key '" + key + "' not found");
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("InputBundle: bad type cast for key '" + key + "'");
        }
    }

    template <typename T>
    bool has(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) return false;
        return std::any_cast<T>(&it->second) != nullptr;
    }

    // Check key existence
    bool contains(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    void remove(const std::string& key) {
        data_.erase(key);
    }

    std::size_t size() const {
        return data_.size();
    }

    std::vector<std::string> keys() const {
        std::vector<std::string> out;
        out.reserve(data_.size());
        for (const auto& [key, _] : data_) {
            out.push_back(key);
        }
        return out;
    }

    void clear() {
        data_.clear();
    }

    std::string describe() const {
        std::ostringstream oss;
        for (const auto& [key, val] : data_) {
            oss << key << " -> " << val.type().name() << "\n";
        }
        return oss.str();
    }

private:
    std::unordered_map<std::string, std::any> data_;
};

#endif  // ANALYSIS_PIPELINE_CONTEXT_INPUT_BUNDLE_H