#include "Scanner.hpp"
#include <fstream>
#include <regex>
#include <unordered_set>
#include <spdlog/spdlog.h>

namespace slate::scanner {

namespace fs = std::filesystem;

std::vector<ScannedFile> Scanner::scan_directory(const fs::path& root) const {
    std::vector<ScannedFile> results;
    
    const std::unordered_set<std::string> supported_extensions = {
        ".c", ".h", ".cpp", ".hpp", ".cxx", ".hxx", ".cc", ".hh",
        ".rs", ".py", ".java", ".cs", ".go", ".ts", ".js", ".jsx", ".tsx"
    };

    for (auto it = fs::recursive_directory_iterator(root);
         it != fs::recursive_directory_iterator();
         ++it) {
        // Skip hidden directories
        if (it->is_directory() && it->path().filename().string().starts_with(".")) {
            it.disable_recursion_pending();
            continue;
        }
        // Skip common build/dependency directories
        if (it->is_directory()) {
            const auto& dirname = it->path().filename().string();
            if (dirname == "node_modules" || dirname == "target" || dirname == "build" ||
                dirname == ".git" || dirname == ".svn" || dirname == "third_party") {
                it.disable_recursion_pending();
                continue;
            }
        }
        
        if (it->is_regular_file()) {
            std::string ext = it->path().extension().string();
            if (supported_extensions.find(ext) != supported_extensions.end()) {
                ScannedFile sf;
                sf.path = it->path();
                sf.language = detect_language(sf.path);
                sf.includes = extract_includes(sf.path);
                results.push_back(std::move(sf));
            }
        }
    }
    
    spdlog::info("Scanner found {} files in {}", results.size(), root.string());
    return results;
}

std::string Scanner::detect_language(const fs::path& path) {
    std::string ext = path.extension().string();
    if (ext == ".c") return "C";
    if (ext == ".h") return "C/C++";
    if (ext == ".cpp" || ext == ".hpp" || ext == ".cxx" || ext == ".hxx" || ext == ".cc" || ext == ".hh") return "C++";
    if (ext == ".rs") return "Rust";
    if (ext == ".py") return "Python";
    if (ext == ".java") return "Java";
    if (ext == ".cs") return "C#";
    if (ext == ".go") return "Go";
    if (ext == ".ts") return "TypeScript";
    if (ext == ".tsx") return "TypeScript React";
    if (ext == ".js" || ext == ".jsx") return "JavaScript";
    return "Unknown";
}

std::vector<std::string> Scanner::extract_includes(const fs::path& path) {
    std::vector<std::string> includes;
    std::ifstream file(path);
    if (!file.is_open()) return includes;

    std::string line;
    // Regex untuk #include "..." atau #include <...>
    std::regex include_re(R"(^\s*#\s*include\s*[<\"]([^>\"]+)[>\"])");
    // Regex untuk Rust use atau mod
    std::regex rust_use_re(R"(^\s*(use|mod)\s+([\w:]+))");
    // Regex untuk Python/JS import
    std::regex import_re(R"(^\s*(import|from)\s+[\"']?([\w./]+))");

    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, include_re)) {
            includes.push_back(match[1].str());
        } else if (std::regex_search(line, match, rust_use_re)) {
            includes.push_back(match[2].str());
        } else if (std::regex_search(line, match, import_re)) {
            includes.push_back(match[2].str());
        }
    }
    return includes;
}

std::vector<ScannedSymbol> Scanner::extract_symbols(const ScannedFile& file) const {
    std::vector<ScannedSymbol> symbols;
    std::ifstream f(file.path);
    if (!f.is_open()) return symbols;

    std::string lang = file.language;
    
    // Regex untuk fungsi C/C++/Rust
    std::regex func_re(R"(^\s*(?:static\s+)?(?:inline\s+)?(?:const\s+)?(?:virtual\s+)?(?:[\w:<>*&]+\s+)+(\w+)\s*\([^)]*\)\s*(?:const\s*)?\{)");
    // Regex untuk class/struct
    std::regex class_re(R"(^\s*(?:class|struct)\s+(\w+))");
    // Regex untuk variabel global (perkiraan)
    std::regex var_re(R"(^\s*(?:static\s+)?(?:const\s+)?(?:[\w:<>*&]+\s+)(\w+)\s*[=;])");
    // Regex untuk macro
    std::regex macro_re(R"(^\s*#\s*define\s+(\w+))");

    std::string line;
    int line_number = 0;
    while (std::getline(f, line)) {
        line_number++;
        std::smatch match;
        
        // Jangan tangkap line yang diawali comment
        if (line.find("//") == 0) continue;
        
        if (std::regex_search(line, match, class_re)) {
            symbols.push_back({match[1].str(), "class", line_number, 0});
        } else if (std::regex_search(line, match, macro_re)) {
            symbols.push_back({match[1].str(), "macro", line_number, 0});
        } else if (std::regex_search(line, match, func_re)) {
            symbols.push_back({match[1].str(), "function", line_number, 0});
        }
    }
    return symbols;
}

} // namespace slate::scanner
