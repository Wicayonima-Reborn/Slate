#include <iostream>
#include <string>
#include "ApplicationContext.hpp"
#include "Scanner.hpp"
#include "Database.hpp"
#include "Graph.hpp"

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

    slate::graph::Graph dep_graph;

    for (const auto& file : files) {
        int file_id = db.insert_file(file.path.string(), file.language);
        
        // Insert symbols
        auto symbols = scanner.extract_symbols(file);
        for (const auto& sym : symbols) {
            int sym_id = db.insert_symbol(sym.name, sym.kind, file_id, sym.line, sym.column);
            total_symbols++;
            
            // Tambahkan node ke graph
            slate::graph::Node node;
            node.id = std::to_string(sym_id);
            node.name = sym.name;
            node.type = slate::graph::NodeType::Symbol;
            dep_graph.add_node(node);
        }

        // Insert dependencies (include relationships sebagai simbol file)
        for (const auto& inc : file.includes) {
            // Cari file yang di-include di database, jika ada buat dependency
            // Untuk sederhana, kita catat sebagai dependency antar file
            // Nanti bisa diperbaiki dengan resolusi simbol yang tepat
            total_deps++;
        }
    }

    // Catat dependency count sebagai estimasi
    // Untuk MVP, dependensi = include count
    // Di masa depan akan dihitung dari graph

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

    std::cout << "Files: " << files << "\n";
    std::cout << "Symbols: " << symbols << "\n";
    std::cout << "Dependencies: " << deps << "\n";
    
    // Circular dependencies akan dihitung dari graph saat kita muat ulang
    std::cout << "Circular Dependencies: 0  (graph analysis pending)\n";

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
