#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "ApplicationContext.hpp"
#include "Scanner.hpp"
#include "Database.hpp"
#include "Graph.hpp"

namespace fs = std::filesystem;

void print_usage() {
    std::cout << "Slate v0.1.0 - Understand your codebase.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  slate scan <directory>   Scan a project directory\n";
    std::cout << "  slate stats <database>   Show statistics\n";
}

int cmd_scan(const std::string& directory) {
    slate::storage::Database db;
    if (!db.open("slate.db")) {
        std::cerr << "Failed to open database\n";
        return 1;
    }

    slate::scanner::Scanner scanner;
    auto files = scanner.scan_directory(directory);

    int session_id = db.start_indexing_session();
    int total_symbols = 0;
    int total_deps = 0;

    // Map: path string -> file_id di database
    std::unordered_map<std::string, int> path_to_file_id;
    // Map: path string -> list of sym_id
    std::unordered_map<std::string, std::vector<int>> path_to_symbol_ids;

    // Pass 1: insert files dan extract symbols
    for (const auto& file : files) {
        int file_id = db.insert_file(file.path.string(), file.language);
        path_to_file_id[file.path.string()] = file_id;

        auto symbols = scanner.extract_symbols(file);
        for (const auto& sym : symbols) {
            int sym_id = db.insert_symbol(sym.name, sym.kind, file_id, sym.line, sym.column);
            path_to_symbol_ids[file.path.string()].push_back(sym_id);
            total_symbols++;
        }
    }

    // Pass 2: bangun dependency dari include
    // Untuk setiap include, kita cari file target di path_to_file_id.
    // Jika ketemu, kita buat dependency dari simbol pertama file source ke simbol pertama file target.
    for (const auto& file : files) {
        const auto& source_symbols = path_to_symbol_ids[file.path.string()];
        if (source_symbols.empty()) continue;

        for (const auto& inc : file.includes) {
            // Cari file target berdasarkan nama include saja (perkiraan)
            // Kita coba cocokkan akhiran path dengan include string
            int target_file_id = -1;
            for (const auto& [path, fid] : path_to_file_id) {
                // Cek apakah path mengandung string include (misal "Graph.hpp" cocok dengan include "Graph.hpp")
                if (path.ends_with(inc) || path.ends_with(inc + ".h") || path.ends_with(inc + ".hpp") ||
                    path.ends_with(inc + ".cpp") || path.ends_with(inc + ".c")) {
                    target_file_id = fid;
                    break;
                }
            }
            if (target_file_id >= 0 && path_to_symbol_ids.count(file.path.string())) {
                const auto& target_symbols = path_to_symbol_ids.begin()->second; // ambil dari map
                // Cari entry yang tepat
                auto it = path_to_symbol_ids.begin();
                for (const auto& [p, syms] : path_to_symbol_ids) {
                    if (path_to_file_id[p] == target_file_id && !syms.empty()) {
                        int source_sym = source_symbols[0];
                        int target_sym = syms[0];
                        db.insert_dependency(source_sym, target_sym, "include");
                        total_deps++;
                        break;
                    }
                }
            }
        }
    }

    db.finish_indexing_session(session_id, "completed");
    
    std::cout << "Scanned " << files.size() << " files\n";
    std::cout << "Found " << total_symbols << " symbols\n";
    std::cout << "Found " << total_deps << " dependencies\n";

    db.close();
    return 0;
}

int cmd_stats(const std::string& db_path) {
    slate::storage::Database db;
    if (!db.open(db_path)) {
        std::cerr << "Failed to open database: " << db_path << "\n";
        return 1;
    }

    int files = db.count_files();
    int symbols = db.count_symbols();
    int deps = db.count_dependencies();

    // Hitung circular dependencies dari graph
    int circular = 0;
    if (deps > 0) {
        // Bangun graph dari dependencies di database
        // Untuk MVP, kita query langsung pakai sqlite3
        // Gunakan method baru: load_all_dependencies
        auto deps_list = db.load_all_dependencies();
        slate::graph::Graph dep_graph;
        for (const auto& dep : deps_list) {
            slate::graph::Node src_node;
            src_node.id = std::to_string(dep.source_symbol_id);
            src_node.name = "";
            src_node.type = slate::graph::NodeType::Symbol;
            dep_graph.add_node(src_node);

            slate::graph::Node tgt_node;
            tgt_node.id = std::to_string(dep.target_symbol_id);
            tgt_node.name = "";
            tgt_node.type = slate::graph::NodeType::Symbol;
            dep_graph.add_node(tgt_node);

            slate::graph::Edge edge;
            edge.from_id = std::to_string(dep.source_symbol_id);
            edge.to_id = std::to_string(dep.target_symbol_id);
            edge.kind = dep.kind;
            dep_graph.add_edge(edge);
        }
        auto cycles = dep_graph.find_all_cycles();
        circular = static_cast<int>(cycles.size());
    }

    std::cout << "Files: " << files << "\n";
    std::cout << "Symbols: " << symbols << "\n";
    std::cout << "Dependencies: " << deps << "\n";
    std::cout << "Circular Dependencies: " << circular << "\n";

    db.close();
    return 0;
}

int main(int argc, char* argv[]) {
    slate::core::ApplicationContext ctx;
    ctx.init_logging();

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "scan") {
        if (argc < 3) {
            std::cerr << "Missing directory argument\n";
            return 1;
        }
        return cmd_scan(argv[2]);
    } else if (command == "stats") {
        std::string db = (argc >= 3) ? argv[2] : "slate.db";
        return cmd_stats(db);
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage();
        return 1;
    }
}
