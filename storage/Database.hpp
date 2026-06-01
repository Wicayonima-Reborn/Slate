#pragma once
#include <string>
#include <memory>
#include <vector>
#include <optional>

struct sqlite3;

namespace slate::storage {

struct FileRecord {
    int id;
    std::string path;
    std::string language;
};

struct SymbolRecord {
    int id;
    std::string name;
    std::string kind;   // function, class, struct, variable...
    int file_id;
    int line;
    int column;
};

struct DependencyRecord {
    int id;
    int source_symbol_id;
    int target_symbol_id;
    std::string kind;   // include, call, inherit...
};

struct IndexingSession {
    int id;
    std::string started_at;
    std::string finished_at;
    std::string status;
};

class Database {
public:
    Database();
    ~Database();

    // Buka / buat database file
    bool open(const std::string& db_path);
    void close();

    // Migrasi skema otomatis
    bool migrate();

    // CRUD minimal untuk MVP
    int insert_file(const std::string& path, const std::string& language);
    int insert_symbol(const std::string& name, const std::string& kind, int file_id, int line, int column);
    int insert_dependency(int source_symbol_id, int target_symbol_id, const std::string& kind);
    int start_indexing_session();
    void finish_indexing_session(int session_id, const std::string& status);

    // Query statistik (untuk slate stats)
    int count_files() const;
    int count_symbols() const;
    int count_dependencies() const;
    int count_circular_dependencies() const; // nanti dihitung oleh graph

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace slate::storage
