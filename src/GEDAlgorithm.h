#ifndef GED_ALGORITHM_H
#define GED_ALGORITHM_H

#include "Graph.h"
#include "EditCosts.h"
#include <vector>
#include <map>
#include <algorithm> // For std::sort, std::min

// Forward declaration if needed, or include
// struct EditCosts;
// class Graph;

namespace ged {

// Structure to hold a potential node mapping and its cost
struct NodeSubstitutionCandidate {
    VertexID u; // Vertex from G1
    VertexID v; // Vertex from G2
    double cost;

    bool operator<(const NodeSubstitutionCandidate& other) const {
        return cost < other.cost;
    }
};


class HeuristicGED {
public:
    HeuristicGED(const EditCosts& costs);

    // Computes GED using a greedy heuristic.
    // G1 is considered the "source" and G2 the "target" for edit operations.
    double compute_distance(const Graph& g1, const Graph& g2);

private:
    EditCosts costs_;

    double get_vertex_substitution_cost(const Graph& g, VertexID v_id, const Label& g1_label) const;
    double get_vertex_substitution_cost(const Graph& g1, VertexID u_id, const Graph& g2, VertexID v_id) const;

    // Helper for edge cost calculation based on a given vertex mapping
    double calculate_edge_costs(
        const Graph& g1,
        const Graph& g2,
        const std::map<VertexID, VertexID>& mapping_g1_to_g2 // u -> v
    );
};

} // namespace ged

#endif // GED_ALGORITHM_H
