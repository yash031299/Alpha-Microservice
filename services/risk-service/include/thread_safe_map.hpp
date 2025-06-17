#pragma once
#include <unordered_map>
#include <shared_mutex>
#include <optional>

template <typename K, typename V>
class ThreadSafeMap {
public:
    void insert(const K& key, const V& value) {
        std::unique_lock lock(mutex_);
        map_[key] = value;
    }

    std::optional<V> get(const K& key) const {
        std::shared_lock lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) return it->second;
        return std::nullopt;
    }

    void erase(const K& key) {
        std::unique_lock lock(mutex_);
        map_.erase(key);
    }

    void forEach(const std::function<void(const K&, const V&)>& fn) const {
        std::shared_lock lock(mutex_);
        for (const auto& [k, v] : map_) fn(k, v);
    }

    size_t size() const {
        std::shared_lock lock(mutex_);
        return map_.size();
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<K, V> map_;
};
