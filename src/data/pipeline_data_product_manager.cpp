#include "data/pipeline_data_product_manager.h"
#include "data/pipeline_data_product_lock.h"

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <algorithm>

// Add or update a single product
void PipelineDataProductManager::addOrUpdate(const std::string& name, std::unique_ptr<PipelineDataProduct> product) {
    if (!product) {
        spdlog::warn("[PipelineDataProductManager] Tried to add/update null product for '{}'", name);
        return;
    }
    std::unique_lock managerLock(managerMutex_);
    product->setName(name);
    products_[name].product = std::move(product);
}

// Add or update multiple products atomically
void PipelineDataProductManager::addOrUpdateMultiple(std::vector<std::pair<std::string, std::unique_ptr<PipelineDataProduct>>>&& products) {
    std::unique_lock managerLock(managerMutex_);
    for (auto& [name, product] : products) {
        if (!product) {
            spdlog::warn("[PipelineDataProductManager] Skipping null product for '{}'", name);
            continue;
        }
        product->setName(name);
        products_[name].product = std::move(product);
    }
}

// Remove a single product by name
void PipelineDataProductManager::remove(const std::string& name) {
    std::unique_lock managerLock(managerMutex_);
    products_.erase(name);
}

// Remove multiple products atomically
void PipelineDataProductManager::removeMultiple(const std::vector<std::string>& names) {
    std::unique_lock managerLock(managerMutex_);
    for (const auto& name : names) {
        products_.erase(name);
    }
}

// Clear all products
void PipelineDataProductManager::clear() {
    std::unique_lock managerLock(managerMutex_);
    products_.clear();
}

// Get all product names
std::vector<std::string> PipelineDataProductManager::getAllNames() const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    names.reserve(products_.size());
    for (const auto& [name, _] : products_) {
        names.push_back(name);
    }
    return names;
}

// Check if a single product exists
bool PipelineDataProductManager::hasProduct(const std::string& name) const {
    std::shared_lock managerLock(managerMutex_);
    return products_.find(name) != products_.end();
}

// Check existence of multiple products
std::vector<bool> PipelineDataProductManager::hasProducts(const std::vector<std::string>& names) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<bool> results;
    results.reserve(names.size());
    for (const auto& name : names) {
        results.push_back(products_.count(name) > 0);
    }
    return results;
}

// Return subset of names that exist
std::vector<std::string> PipelineDataProductManager::getExistingProducts(const std::vector<std::string>& names) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> existing;
    for (const auto& name : names) {
        if (products_.count(name)) existing.push_back(name);
    }
    return existing;
}

// Checkout a single product for reading (shared lock)
PipelineDataProductLock PipelineDataProductManager::checkoutRead(const std::string& name) {
    std::shared_lock managerLock(managerMutex_);
    auto it = products_.find(name);
    if (it == products_.end()) {
        throw std::runtime_error("Product not found: " + name);
    }
    ProductEntry& entry = it->second;

    managerLock.unlock();

    std::shared_lock productLock(entry.mutex);
    return PipelineDataProductLock(entry.product.get(), std::move(productLock));
}

// Checkout a single product for writing (unique lock)
PipelineDataProductLock PipelineDataProductManager::checkoutWrite(const std::string& name) {
    std::shared_lock managerLock(managerMutex_);
    auto it = products_.find(name);
    if (it == products_.end()) {
        throw std::runtime_error("Product not found: " + name);
    }
    ProductEntry& entry = it->second;

    managerLock.unlock();

    std::unique_lock productLock(entry.mutex);
    return PipelineDataProductLock(entry.product.get(), std::move(productLock));
}

// Checkout multiple products for reading (shared locks)
std::vector<PipelineDataProductLock> PipelineDataProductManager::checkoutReadMultiple(const std::vector<std::string>& names) {
    std::vector<std::string> sortedNames = names;
    std::sort(sortedNames.begin(), sortedNames.end());

    std::vector<PipelineDataProductLock> handles;
    handles.reserve(sortedNames.size());

    std::shared_lock managerLock(managerMutex_);

    std::vector<ProductEntry*> entries;
    entries.reserve(sortedNames.size());
    for (const auto& name : sortedNames) {
        auto it = products_.find(name);
        if (it == products_.end()) {
            throw std::runtime_error("Product not found: " + name);
        }
        entries.push_back(&it->second);
    }

    managerLock.unlock();

    for (auto* entry : entries) {
        std::shared_lock productLock(entry->mutex);
        // You have to be careful with adding these to vectors; the vector cannot
        // construct new PipelineDataProducts as PipelineDataProductManager is the 
        // only friend class that is allowed to do so. So we have to make them here
        // then push them back (cannot do emplace back or similar)
        PipelineDataProductLock lock(entry->product.get(), std::move(productLock)); // local construction
        handles.push_back(std::move(lock));                                        // then push_back move
    }

    return handles;
}

// Checkout multiple products for writing (unique locks)
std::vector<PipelineDataProductLock> PipelineDataProductManager::checkoutWriteMultiple(const std::vector<std::string>& names) {
    std::vector<std::string> sortedNames = names;
    std::sort(sortedNames.begin(), sortedNames.end());

    std::vector<PipelineDataProductLock> handles;
    handles.reserve(sortedNames.size());

    std::shared_lock managerLock(managerMutex_);

    std::vector<ProductEntry*> entries;
    entries.reserve(sortedNames.size());
    for (const auto& name : sortedNames) {
        auto it = products_.find(name);
        if (it == products_.end()) {
            throw std::runtime_error("Product not found: " + name);
        }
        entries.push_back(&it->second);
    }

    managerLock.unlock();

    for (auto* entry : entries) {
        std::unique_lock productLock(entry->mutex);
        // You have to be careful with adding these to vectors; the vector cannot
        // construct new PipelineDataProducts as PipelineDataProductManager is the 
        // only friend class that is allowed to do so. So we have to make them here
        // then push them back (cannot do emplace back or similar)
        PipelineDataProductLock lock(entry->product.get(), std::move(productLock)); // local construction
        handles.push_back(std::move(lock));                                        // then push_back move
    }

    return handles;
}

nlohmann::json PipelineDataProductManager::serializeAll() const {
    nlohmann::json output;

    std::shared_lock<std::shared_mutex> lock(managerMutex_);
    for (const auto& [name, entry] : products_) {
        std::shared_lock<std::shared_mutex> entryLock(entry.mutex);
        output[name] = entry.product->serializeToJson();
    }

    return output;
}

// Get all unique tags used across all products
std::unordered_set<std::string> PipelineDataProductManager::getAllTags() const {
    std::shared_lock managerLock(managerMutex_);
    std::unordered_set<std::string> result;
    for (const auto& [_, entry] : products_) {
        const auto& tags = entry.product->getTags();
        result.insert(tags.begin(), tags.end());
    }
    return result;
}

// Remove all products that contain the given tag
void PipelineDataProductManager::removeByTag(const std::string& tag) {
    std::unique_lock managerLock(managerMutex_);
    for (auto it = products_.begin(); it != products_.end(); ) {
        if (it->second.product->hasTag(tag)) {
            it = products_.erase(it);
        } else {
            ++it;
        }
    }
}

// Remove all products that DO NOT contain the given tag
void PipelineDataProductManager::removeExcludingTag(const std::string& tag) {
    std::unique_lock managerLock(managerMutex_);
    for (auto it = products_.begin(); it != products_.end(); ) {
        if (!it->second.product->hasTag(tag)) {
            it = products_.erase(it);
        } else {
            ++it;
        }
    }
}

// Get names of products with the specified tag
std::vector<std::string> PipelineDataProductManager::getNamesWithTag(const std::string& tag) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, entry] : products_) {
        if (entry.product->hasTag(tag)) {
            names.push_back(name);
        }
    }
    return names;
}

// Remove products with ANY of the specified tags
void PipelineDataProductManager::removeByTags(const std::unordered_set<std::string>& tags) {
    std::unique_lock managerLock(managerMutex_);
    for (auto it = products_.begin(); it != products_.end(); ) {
        const auto& prodTags = it->second.product->getTags();
        if (std::any_of(tags.begin(), tags.end(),
                        [&](const std::string& tag) { return prodTags.count(tag); })) {
            it = products_.erase(it);
        } else {
            ++it;
        }
    }
}

// Remove products that DO NOT have ANY of the specified tags
void PipelineDataProductManager::removeExcludingTags(const std::unordered_set<std::string>& tags) {
    std::unique_lock managerLock(managerMutex_);
    for (auto it = products_.begin(); it != products_.end(); ) {
        const auto& prodTags = it->second.product->getTags();
        if (std::none_of(tags.begin(), tags.end(),
                         [&](const std::string& tag) { return prodTags.count(tag); })) {
            it = products_.erase(it);
        } else {
            ++it;
        }
    }
}

// Get names of products with ANY of the specified tags
std::vector<std::string> PipelineDataProductManager::getNamesWithAnyTags(const std::unordered_set<std::string>& tags) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, entry] : products_) {
        const auto& prodTags = entry.product->getTags();
        if (std::any_of(tags.begin(), tags.end(),
                        [&](const std::string& tag) { return prodTags.count(tag); })) {
            names.push_back(name);
        }
    }
    return names;
}

// Get names of products that contain ALL of the specified tags
std::vector<std::string> PipelineDataProductManager::getNamesWithAllTags(const std::unordered_set<std::string>& tags) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, entry] : products_) {
        const auto& prodTags = entry.product->getTags();
        if (std::all_of(tags.begin(), tags.end(),
                        [&](const std::string& tag) { return prodTags.count(tag); })) {
            names.push_back(name);
        }
    }
    return names;
}

// Get names of products that have exactly the specified tag set (no extras)
std::vector<std::string> PipelineDataProductManager::getNamesWithExactTags(const std::unordered_set<std::string>& tags) const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, entry] : products_) {
        const auto& prodTags = entry.product->getTags();
        if (prodTags == tags) {
            names.push_back(name);
        }
    }
    return names;
}

// Get names of products that have no tags at all
std::vector<std::string> PipelineDataProductManager::getNamesWithNoTags() const {
    std::shared_lock managerLock(managerMutex_);
    std::vector<std::string> names;
    for (const auto& [name, entry] : products_) {
        if (entry.product->getTags().empty()) {
            names.push_back(name);
        }
    }
    return names;
}