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

/**
 * InputBundle is a heterogeneous, optionally owning input container designed for
 * passing per-event data to pipeline stages. It supports both reference-based
 * (non-owning) and value-based (owning) semantics using type-erased storage.
 *
 * Typical use is setRef/getRef for fast, zero-copy input access. For flexibility,
 * setValue/getValue allow copying data into the bundle explicitly.
 */
class InputBundle {
public:
    InputBundle() = default;
    ~InputBundle() = default;

    InputBundle(const InputBundle&) = delete;
    InputBundle& operator=(const InputBundle&) = delete;

    InputBundle(InputBundle&&) = default;
    InputBundle& operator=(InputBundle&&) = default;

    // --- Reference-based (non-owning) interface ---

    template <typename T>
    void setRef(const std::string& key, T& value) {
        data_[key] = std::ref(value);
    }

    template <typename T>
    T& getRef(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("InputBundle: key '" + key + "' not found");
        }
        try {
            return std::any_cast<std::reference_wrapper<T>>(it->second).get();
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("InputBundle: bad type cast for key '" + key + "' (ref)");
        }
    }

    // --- Value-based (owning) interface ---

    template <typename T>
    void setValue(const std::string& key, T value) {
        data_[key] = std::move(value);
    }

    template <typename T>
    T& getValue(const std::string& key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("InputBundle: key '" + key + "' not found");
        }
        try {
            return std::any_cast<T&>(it->second);
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("InputBundle: bad type cast for key '" + key + "' (value)");
        }
    }

    template <typename T>
    const T& getValue(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("InputBundle: key '" + key + "' not found");
        }
        try {
            return std::any_cast<const T&>(it->second);
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("InputBundle: bad type cast for key '" + key + "' (value)");
        }
    }

    // --- Common operations ---

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

// For stream/logging use
inline std::ostream& operator<<(std::ostream& os, const InputBundle& bundle) {
    return os << bundle.describe();
}

#endif  // ANALYSIS_PIPELINE_CONTEXT_INPUT_BUNDLE_H
