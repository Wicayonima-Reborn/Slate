#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace slate::scanner {

struct ScannedFile {
    std::filesystem::path path;
    std::string language;
    std::vector<std::string> includes; // file yang di-include
};

struct ScannedSymbol {
    std::string name;
    std::string kind;   // function, class, struct, variable, macro
    int line;
    int column;
};

class Scanner {
public:
    Scanner() = default;

    // Pindai direktori, kembalikan daftar file yang terdeteksi
    std::vector<ScannedFile> scan_directory(const std::filesystem::path& root) const;

    // Ekstrak simbol dari sebuah file
    std::vector<ScannedSymbol> extract_symbols(const ScannedFile& file) const;

    // Deteksi bahasa dari ekstensi file
    static std::string detect_language(const std::filesystem::path& path);

    // Ekstrak #include / use statement
    static std::vector<std::string> extract_includes(const std::filesystem::path& path);
};

} // namespace slate::scanner
