#include "Database.hpp"
#include <sqlite3.h>
#include <spdlog/spdlog.h>

namespace slate::storage {

struct Database::Impl {
    sqlite3* db = nullptr;
};

Database::Database() : pimpl_(std::make_unique<Impl>()) {}

Database::~Database() {
    close();
}

bool Database::open(const std::string& db_path) {
    if (pimpl_->db) {
        spdlog::error("Database already open");
        return false;
    }
    int rc = sqlite3_open(db_path.c_str(), &pimpl_->db);
    if (rc != SQLITE_OK) {
        spdlog::error("Cannot open database: {}", sqlite3_errmsg(pimpl_->db));
        return false;
    }
    spdlog::info("Database opened: {}", db_path);
    return migrate();
}

void Database::close() {
    if (pimpl_->db) {
        sqlite3_close(pimpl_->db);
        pimpl_->db = nullptr;
        spdlog::info("Database closed");
    }
}

bool Database::migrate() {
    const char* create_files = R"(
        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE,
            language TEXT
        );
    )";
    const char* create_symbols = R"(
        CREATE TABLE IF NOT EXISTS symbols (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            kind TEXT NOT NULL,
            file_id INTEGER NOT NULL,
            line INTEGER,
            column INTEGER,
            FOREIGN KEY(file_id) REFERENCES files(id)
        );
    )";
    const char* create_dependencies = R"(
        CREATE TABLE IF NOT EXISTS dependencies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            source_symbol_id INTEGER NOT NULL,
            target_symbol_id INTEGER NOT NULL,
            kind TEXT NOT NULL,
            FOREIGN KEY(source_symbol_id) REFERENCES symbols(id),
            FOREIGN KEY(target_symbol_id) REFERENCES symbols(id)
        );
    )";
    const char* create_sessions = R"(
        CREATE TABLE IF NOT EXISTS indexing_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            started_at TEXT DEFAULT (datetime('now')),
            finished_at TEXT,
            status TEXT DEFAULT 'running'
        );
    )";

    auto exec = [this](const char* sql) -> bool {
        char* errmsg = nullptr;
        if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            spdlog::error("Migration error: {}", errmsg);
            sqlite3_free(errmsg);
            return false;
        }
        return true;
    };

    if (!exec(create_files)) return false;
    if (!exec(create_symbols)) return false;
    if (!exec(create_dependencies)) return false;
    if (!exec(create_sessions)) return false;

    spdlog::info("Database migration completed");
    return true;
}

int Database::insert_file(const std::string& path, const std::string& language) {
    const char* sql = "INSERT OR IGNORE INTO files (path, language) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, language.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(pimpl_->db));
}

int Database::insert_symbol(const std::string& name, const std::string& kind, int file_id, int line, int column) {
    const char* sql = "INSERT INTO symbols (name, kind, file_id, line, column) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, kind.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, file_id);
    sqlite3_bind_int(stmt, 4, line);
    sqlite3_bind_int(stmt, 5, column);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(pimpl_->db));
}

int Database::insert_dependency(int source_symbol_id, int target_symbol_id, const std::string& kind) {
    const char* sql = "INSERT INTO dependencies (source_symbol_id, target_symbol_id, kind) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, source_symbol_id);
    sqlite3_bind_int(stmt, 2, target_symbol_id);
    sqlite3_bind_text(stmt, 3, kind.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(pimpl_->db));
}

int Database::start_indexing_session() {
    const char* sql = "INSERT INTO indexing_sessions DEFAULT VALUES;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(pimpl_->db));
}

void Database::finish_indexing_session(int session_id, const std::string& status) {
    const char* sql = "UPDATE indexing_sessions SET finished_at = datetime('now'), status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, session_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int Database::count_files() const {
    const char* sql = "SELECT COUNT(*) FROM files;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int Database::count_symbols() const {
    const char* sql = "SELECT COUNT(*) FROM symbols;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int Database::count_dependencies() const {
    const char* sql = "SELECT COUNT(*) FROM dependencies;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int Database::count_circular_dependencies() const {
    // Akan dihitung oleh modul graph, bukan di sini
    return -1;
}

} // namespace slate::storage
