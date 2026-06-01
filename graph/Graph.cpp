#include "Graph.hpp"
#include <algorithm>
#include <functional>

namespace slate::graph {

void Graph::add_node(const Node& node) {
    nodes_[node.id] = node;
}

std::optional<Node> Graph::get_node(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it != nodes_.end()) return it->second;
    return std::nullopt;
}

void Graph::add_edge(const Edge& edge) {
    edges_.push_back(edge);
    adjacency_out_[edge.from_id].push_back(edge);
    adjacency_in_[edge.to_id].push_back(edge);
}

std::vector<Edge> Graph::get_edges_from(const std::string& id) const {
    auto it = adjacency_out_.find(id);
    if (it != adjacency_out_.end()) return it->second;
    return {};
}

std::vector<Edge> Graph::get_edges_to(const std::string& id) const {
    auto it = adjacency_in_.find(id);
    if (it != adjacency_in_.end()) return it->second;
    return {};
}

size_t Graph::node_count() const { return nodes_.size(); }
size_t Graph::edge_count() const { return edges_.size(); }

bool Graph::has_cycle() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;
    for (const auto& [id, _] : nodes_) {
        if (visited.find(id) == visited.end()) {
            if (dfs_has_cycle(id, visited, recursion_stack)) return true;
        }
    }
    return false;
}

bool Graph::dfs_has_cycle(const std::string& node,
                          std::unordered_set<std::string>& visited,
                          std::unordered_set<std::string>& recursion_stack) const {
    visited.insert(node);
    recursion_stack.insert(node);
    auto it = adjacency_out_.find(node);
    if (it != adjacency_out_.end()) {
        for (const auto& edge : it->second) {
            const auto& neighbor = edge.to_id;
            if (recursion_stack.find(neighbor) != recursion_stack.end()) return true;
            if (visited.find(neighbor) == visited.end()) {
                if (dfs_has_cycle(neighbor, visited, recursion_stack)) return true;
            }
        }
    }
    recursion_stack.erase(node);
    return false;
}

std::vector<std::vector<std::string>> Graph::find_all_cycles() const {
    std::vector<std::vector<std::string>> cycles;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;
    std::vector<std::string> path;

    // Deklarasi eksplisit std::function agar lambda bisa rekursif
    std::function<void(const std::string&)> dfs = [&](const std::string& node) {
        visited.insert(node);
        recursion_stack.insert(node);
        path.push_back(node);

        auto it = adjacency_out_.find(node);
        if (it != adjacency_out_.end()) {
            for (const auto& edge : it->second) {
                const auto& neighbor = edge.to_id;
                if (recursion_stack.find(neighbor) != recursion_stack.end()) {
                    // Cycle ditemukan
                    auto start = std::find(path.begin(), path.end(), neighbor);
                    if (start != path.end()) {
                        cycles.emplace_back(start, path.end());
                        cycles.back().push_back(neighbor);
                    }
                } else if (visited.find(neighbor) == visited.end()) {
                    dfs(neighbor);
                }
            }
        }

        path.pop_back();
        recursion_stack.erase(node);
    };

    for (const auto& [id, _] : nodes_) {
        if (visited.find(id) == visited.end()) {
            dfs(id);
        }
    }
    return cycles;
}

} // namespace slate::graph
