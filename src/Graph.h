#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm> // For std::remove

// Using integers for vertex IDs for simplicity and efficiency.
// Labels can be strings or any other comparable type.
using VertexID = int;
using Label = std::string; // Or a generic template type later if needed

struct Edge {
    VertexID to;
    Label label; // Edge label, if any. For GED, this might be less common than vertex labels.
                 // Often edge edit operations are just add/delete, or substitute based on endpoints.
                 // Let's include it for completeness, default to empty if unused.

    // For ordering in sets/maps if needed
    bool operator<(const Edge& other) const {
        if (to != other.to) {
            return to < other.to;
        }
        return label < other.label;
    }
};

class Graph {
public:
    Graph();

    // Vertex operations
    bool add_vertex(VertexID id, const Label& label = "");
    bool remove_vertex(VertexID id); // This must also remove incident edges
    bool set_vertex_label(VertexID id, const Label& label);
    Label get_vertex_label(VertexID id) const;
    bool has_vertex(VertexID id) const;
    std::set<VertexID> get_vertices() const;
    size_t num_vertices() const;

    // Edge operations
    // Assuming undirected graphs by default: adding (u,v) implies (v,u) is also considered.
    // For implementation, we might store only one direction if labels are symmetric.
    // If directed, client code must add both (u,v) and (v,u) if a symmetric edge is desired.
    // Let's make it explicit: add_edge is for directed, or for one direction of undirected.
    // For GED on undirected graphs, we usually match (u,v) with (x,y) or (y,x).
    bool add_edge(VertexID from, VertexID to, const Label& label = "");
    bool remove_edge(VertexID from, VertexID to); // Removes specific directed edge
    bool set_edge_label(VertexID from, VertexID to, const Label& label);
    Label get_edge_label(VertexID from, VertexID to) const; // Gets label of specific directed edge
    bool has_edge(VertexID from, VertexID to) const;
    std::set<Edge> get_edges_from(VertexID from) const; // Gets outgoing edges
    std::vector<std::pair<VertexID, VertexID>> get_all_edges() const; // Returns pairs of connected vertices
    size_t num_edges() const; // Counts unique undirected edges if symmetric, or directed edges

    // GED operations - these are conceptual, actual implementation will be in a GED algorithm class
    // double cost_vertex_insertion(VertexID id, const Label& label);
    // double cost_vertex_deletion(VertexID id);
    // double cost_vertex_substitution(VertexID id, const Label& new_label);
    // double cost_edge_insertion(VertexID from, VertexID to, const Label& label);
    // double cost_edge_deletion(VertexID from, VertexID to);
    // double cost_edge_substitution(VertexID from, VertexID to, const Label& new_label);

    void print_graph() const; // For debugging

private:
    // Adjacency list: map from VertexID to a set of its outgoing Edges
    std::map<VertexID, std::set<Edge>> adj_list_;
    // Map from VertexID to its label
    std::map<VertexID, Label> vertex_labels_;
    // To keep track of the number of unique edges (especially for undirected graphs if we store directed edges)
    // For now, num_edges_ will count directed edges. If graph is treated as undirected,
    // the GED algorithm would need to handle symmetry.
    size_t num_edges_;

    // Helper to find an edge for modification or retrieval
    std::set<Edge>::iterator find_edge_iterator(VertexID from, VertexID to);
    const std::set<Edge>::const_iterator find_edge_iterator(VertexID from, VertexID to) const;
};

#endif // GRAPH_H
