#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace slate::graph {

enum class NodeType {
    File,
    Symbol,
    Module,
    Library
};

struct Node {
    std::string id;
    std::string name;
    NodeType type;
};

struct Edge {
    std::string from_id;
    std::string to_id;
    std::string kind; // "include", "call", "inherit", dll
};

class Graph {
public:
    Graph() = default;

    void add_node(const Node& node);
    std::optional<Node> get_node(const std::string& id) const;
    void add_edge(const Edge& edge);
    std::vector<Edge> get_edges_from(const std::string& id) const;
    std::vector<Edge> get_edges_to(const std::string& id) const;

    size_t node_count() const;
    size_t edge_count() const;

    // Deteksi cycle sederhana pakai DFS
    bool has_cycle() const;

    // Kembalikan daftar cycle (sederhana, bisa diperbaiki nanti)
    std::vector<std::vector<std::string>> find_all_cycles() const;

private:
    std::unordered_map<std::string, Node> nodes_;
    std::vector<Edge> edges_;
    std::unordered_map<std::string, std::vector<Edge>> adjacency_out_;
    std::unordered_map<std::string, std::vector<Edge>> adjacency_in_;

    bool dfs_has_cycle(const std::string& node,
                       std::unordered_set<std::string>& visited,
                       std::unordered_set<std::string>& recursion_stack) const;
};

} // namespace slate::graph
