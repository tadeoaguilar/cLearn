#include "infra/connection_pool.hpp"

#include "domain/errors.hpp"

namespace tasks::infra {

ConnectionPool::ConnectionPool(std::string url, std::size_t size) : url_{std::move(url)}, size_{size} {
    // Open one connection eagerly so misconfiguration fails fast at startup.
    idle_.push_back(open());
    open_count_ = 1;
}

std::unique_ptr<pqxx::connection> ConnectionPool::open() {
    try {
        return std::make_unique<pqxx::connection>(url_);
    } catch (const pqxx::broken_connection& e) {
        throw StorageUnavailableError(std::string("cannot connect to database: ") + e.what());
    }
}

ConnectionPool::Lease ConnectionPool::acquire(std::chrono::milliseconds timeout) {
    std::unique_lock lock(mutex_);
    while (true) {
        if (!idle_.empty()) {
            auto conn = std::move(idle_.back());
            idle_.pop_back();
            if (conn->is_open()) return Lease{*this, std::move(conn)};
            --open_count_; // dead connection: drop it and try again
            continue;
        }
        if (open_count_ < size_) {
            ++open_count_; // reserve a slot, then connect WITHOUT holding the lock
            lock.unlock();
            try {
                return Lease{*this, open()};
            } catch (...) {
                lock.lock();
                --open_count_;
                throw;
            }
        }
        if (available_.wait_for(lock, timeout) == std::cv_status::timeout && idle_.empty() && open_count_ >= size_)
            throw StorageUnavailableError("timed out waiting for a database connection");
    }
}

void ConnectionPool::release(std::unique_ptr<pqxx::connection> conn) {
    {
        std::lock_guard lock(mutex_);
        if (conn->is_open()) idle_.push_back(std::move(conn));
        else --open_count_; // broken (e.g. server restarted): a fresh one will be opened on demand
    }
    available_.notify_one();
}

} // namespace tasks::infra
