#include "Graph.h"
#include <iostream>
#include <stdexcept> // For std::out_of_range

Graph::Graph() : num_edges_(0) {}

// Vertex operations
bool Graph::add_vertex(VertexID id, const Label& label) {
    if (vertex_labels_.count(id)) {
        return false; // Vertex already exists
    }
    vertex_labels_[id] = label;
    adj_list_[id] = std::set<Edge>(); // Initialize adjacency list for this vertex
    return true;
}

bool Graph::remove_vertex(VertexID id) {
    if (!vertex_labels_.count(id)) {
        return false; // Vertex does not exist
    }
    vertex_labels_.erase(id);

    // Remove all edges connected to this vertex
    // 1. Outgoing edges from this vertex
    if (adj_list_.count(id)) {
        num_edges_ -= adj_list_[id].size();
        adj_list_.erase(id);
    }

    // 2. Incoming edges to this vertex
    for (auto& pair : adj_list_) {
        std::set<Edge>& edges = pair.second;
        for (auto it = edges.begin(); it != edges.end(); ) {
            if (it->to == id) {
                it = edges.erase(it);
                num_edges_--; // Decrement edge count
            } else {
                ++it;
            }
        }
    }
    return true;
}

bool Graph::set_vertex_label(VertexID id, const Label& label) {
    if (!vertex_labels_.count(id)) {
        return false; // Vertex does not exist
    }
    vertex_labels_[id] = label;
    return true;
}

Label Graph::get_vertex_label(VertexID id) const {
    auto it = vertex_labels_.find(id);
    if (it == vertex_labels_.end()) {
        throw std::out_of_range("Vertex ID not found in get_vertex_label");
    }
    return it->second;
}

bool Graph::has_vertex(VertexID id) const {
    return vertex_labels_.count(id);
}

std::set<VertexID> Graph::get_vertices() const {
    std::set<VertexID> vertices;
    for (const auto& pair : vertex_labels_) {
        vertices.insert(pair.first);
    }
    return vertices;
}

size_t Graph::num_vertices() const {
    return vertex_labels_.size();
}

// Edge operations
bool Graph::add_edge(VertexID from, VertexID to, const Label& label) {
    if (!vertex_labels_.count(from) || !vertex_labels_.count(to)) {
        return false; // One or both vertices do not exist
    }
    // Check if edge already exists
    for (const auto& edge : adj_list_[from]) {
        if (edge.to == to) {
            return false; // Edge already exists
        }
    }
    adj_list_[from].insert({to, label});
    num_edges_++;
    return true;
}

bool Graph::remove_edge(VertexID from, VertexID to) {
    if (!adj_list_.count(from)) {
        return false; // 'from' vertex does not exist or has no outgoing edges
    }
    std::set<Edge>& edges = adj_list_[from];
    for (auto it = edges.begin(); it != edges.end(); ++it) {
        if (it->to == to) {
            edges.erase(it);
            num_edges_--;
            return true;
        }
    }
    return false; // Edge not found
}

std::set<Edge>::iterator Graph::find_edge_iterator(VertexID from, VertexID to) {
    if (!adj_list_.count(from)) {
        return std::set<Edge>::iterator(); // Or throw, or return adj_list_[from].end() after ensuring 'from' exists
    }
    auto& edges = adj_list_.at(from); // Use .at() to throw if 'from' is not there (should be caught by has_vertex usually)
    return std::find_if(edges.begin(), edges.end(),
                        [to](const Edge& edge){ return edge.to == to; });
}

const std::set<Edge>::const_iterator Graph::find_edge_iterator(VertexID from, VertexID to) const {
    if (!adj_list_.count(from)) {
        // If 'from' vertex doesn't exist in adj_list_ (e.g., isolated vertex or not found)
        // we need a way to return an end iterator for an empty set or handle appropriately.
        // This scenario implies the edge cannot exist.
        // A common pattern is to return the end iterator of where it would be if 'from' existed.
        // However, since adj_list_ might not contain 'from', we can't directly get adj_list_.at(from).end().
        // For safety, if 'from' isn't in adj_list_, it means no outgoing edges, so edge (from,to) can't exist.
        // We can return a default-constructed iterator, or the end iterator of an empty set.
        // Let's find 'from' first.
        auto it_adj = adj_list_.find(from);
        if (it_adj == adj_list_.end()) {
            static const std::set<Edge> empty_set; // To return .end() from
            return empty_set.end();
        }
        const auto& edges = it_adj->second;
        return std::find_if(edges.begin(), edges.end(),
                            [to](const Edge& edge){ return edge.to == to; });
    }
}


bool Graph::set_edge_label(VertexID from, VertexID to, const Label& label) {
    if (!adj_list_.count(from)) {
        return false;
    }
    auto& edges = adj_list_.at(from);
    auto it = std::find_if(edges.begin(), edges.end(),
                           [to](const Edge& edge){ return edge.to == to; });
    if (it != edges.end()) {
        // Edge exists, modify its label.
        // Since std::set stores const elements, we can't modify in place.
        // We must remove and re-insert.
        Edge new_edge = {it->to, label};
        edges.erase(it);
        edges.insert(new_edge);
        return true;
    }
    return false; // Edge not found
}

Label Graph::get_edge_label(VertexID from, VertexID to) const {
    auto it_adj = adj_list_.find(from);
    if (it_adj == adj_list_.end()) {
        throw std::out_of_range("Edge not found in get_edge_label (source vertex does not exist or has no edges)");
    }
    const auto& edges = it_adj->second;
    auto it = std::find_if(edges.begin(), edges.end(),
                           [to](const Edge& edge){ return edge.to == to; });

    if (it != edges.end()) {
        return it->label;
    }
    throw std::out_of_range("Edge not found in get_edge_label");
}

bool Graph::has_edge(VertexID from, VertexID to) const {
    auto it_adj = adj_list_.find(from);
    if (it_adj == adj_list_.end()) {
        return false;
    }
    const auto& edges = it_adj->second;
    return std::any_of(edges.begin(), edges.end(),
                       [to](const Edge& edge){ return edge.to == to; });
}

std::set<Edge> Graph::get_edges_from(VertexID from) const {
    auto it = adj_list_.find(from);
    if (it == adj_list_.end()) {
        return std::set<Edge>(); // Return empty set if vertex has no outgoing edges or doesn't exist
    }
    return it->second;
}

std::vector<std::pair<VertexID, VertexID>> Graph::get_all_edges() const {
    std::vector<std::pair<VertexID, VertexID>> all_edges;
    for (const auto& pair : adj_list_) {
        VertexID from_node = pair.first;
        for (const auto& edge : pair.second) {
            all_edges.push_back({from_node, edge.to});
        }
    }
    return all_edges;
}

size_t Graph::num_edges() const {
    return num_edges_;
}

void Graph::print_graph() const {
    std::cout << "Graph (" << num_vertices() << " vertices, " << num_edges_ << " edges):" << std::endl;
    for (const auto& v_pair : vertex_labels_) {
        VertexID u = v_pair.first;
        std::cout << "Vertex " << u << " (Label: \"" << v_pair.second << "\")";
        if (adj_list_.count(u) && !adj_list_.at(u).empty()) {
            std::cout << " -> Edges: ";
            for (const auto& edge : adj_list_.at(u)) {
                std::cout << "[" << edge.to << " (Label: \"" << edge.label << "\")] ";
            }
        }
        std::cout << std::endl;
    }
}
