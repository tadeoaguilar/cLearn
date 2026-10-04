// A fixed-size pool of PostgreSQL connections.
//
// pqxx::connection is NOT thread-safe, and opening one costs a TCP + auth round
// trip. The HTTP server handles requests on several threads, so each request
// borrows ("leases") a connection and returns it automatically (RAII) when done.
//
//   auto lease = pool.acquire();     // blocks until a connection is free
//   pqxx::work tx{*lease};           // use it
//                                    // ~Lease() gives it back
#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <pqxx/pqxx>

namespace tasks::infra {

class ConnectionPool {
public:
    ConnectionPool(std::string url, std::size_t size);
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    class Lease {
    public:
        Lease(ConnectionPool& pool, std::unique_ptr<pqxx::connection> conn) : pool_{&pool}, conn_{std::move(conn)} {}
        Lease(Lease&& o) noexcept = default;
        Lease& operator=(Lease&&) = delete;
        ~Lease() {
            if (conn_) pool_->release(std::move(conn_));
        }
        pqxx::connection& operator*() const { return *conn_; }
        pqxx::connection* operator->() const { return conn_.get(); }

    private:
        ConnectionPool* pool_;
        std::unique_ptr<pqxx::connection> conn_;
    };

    // Throws StorageUnavailableError if no connection frees up within `timeout`
    // or a new connection can't be opened.
    Lease acquire(std::chrono::milliseconds timeout = std::chrono::seconds(5));

    std::size_t size() const { return size_; }

private:
    void release(std::unique_ptr<pqxx::connection> conn);
    std::unique_ptr<pqxx::connection> open();

    std::string url_;
    std::size_t size_;
    std::mutex mutex_;
    std::condition_variable available_;
    std::vector<std::unique_ptr<pqxx::connection>> idle_;
    std::size_t open_count_ = 0; // idle + leased
};

} // namespace tasks::infra
