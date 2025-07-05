#include "GEDAlgorithm.h"
#include <set>
#include <iostream> // For debugging (optional)

namespace ged {

HeuristicGED::HeuristicGED(const EditCosts& costs) : costs_(costs) {}

double HeuristicGED::get_vertex_substitution_cost(const Graph& g, VertexID v_id, const Label& g1_label) const {
    if (!g.has_vertex(v_id)) {
        // This case should ideally not be reached if logic is correct
        return costs_.vertex_insertion_cost; // Or some other penalty
    }
    return (g.get_vertex_label(v_id) == g1_label) ? 0.0 : costs_.vertex_substitution_cost;
}

double HeuristicGED::get_vertex_substitution_cost(const Graph& g1, VertexID u_id, const Graph& g2, VertexID v_id) const {
    if (!g1.has_vertex(u_id) || !g2.has_vertex(v_id)) {
        // Should not happen with valid IDs
        throw std::runtime_error("Invalid vertex ID in get_vertex_substitution_cost");
    }
    return (g1.get_vertex_label(u_id) == g2.get_vertex_label(v_id)) ? 0.0 : costs_.vertex_substitution_cost;
}


double HeuristicGED::calculate_edge_costs(
    const Graph& g1,
    const Graph& g2,
    const std::map<VertexID, VertexID>& mapping_g1_to_g2) {

    double current_edge_costs = 0.0;
    std::set<std::pair<VertexID, VertexID>> g2_mapped_edges; // Edges in G2 that correspond to a G1 edge

    // Iterate over edges in G1
    for (VertexID u1 : g1.get_vertices()) {
        for (const auto& edge1 : g1.get_edges_from(u1)) {
            VertexID v1 = edge1.to; // u1 -> v1 is an edge in G1

            auto map_u1_it = mapping_g1_to_g2.find(u1);
            auto map_v1_it = mapping_g1_to_g2.find(v1);

            if (map_u1_it != mapping_g1_to_g2.end() && map_v1_it != mapping_g1_to_g2.end()) {
                // Both u1 and v1 are mapped to some vertices in G2
                VertexID u2 = map_u1_it->second;
                VertexID v2 = map_v1_it->second;

                // Check if the corresponding edge u2 -> v2 exists in G2
                // Assuming edge labels must also match if they are relevant (current Graph struct has edge labels)
                bool g2_has_mapped_edge = false;
                if (g2.has_edge(u2,v2)) {
                    // If edge labels were relevant for substitution, compare g1.get_edge_label(u1,v1) and g2.get_edge_label(u2,v2)
                    // For now, if the mapped edge exists, cost is 0.
                    g2_has_mapped_edge = true;
                    g2_mapped_edges.insert({u2, v2}); // Store directed edge
                }

                if (!g2_has_mapped_edge) {
                    current_edge_costs += costs_.edge_deletion_cost; // G1 edge (u1,v1) deleted as (u2,v2) does not exist
                }
            } else {
                // u1 or v1 (or both) were deleted (not mapped). So edge (u1,v1) is deleted.
                current_edge_costs += costs_.edge_deletion_cost;
            }
        }
    }

    // Iterate over edges in G2 to account for insertions
    for (VertexID u2 : g2.get_vertices()) {
        for (const auto& edge2 : g2.get_edges_from(u2)) {
            VertexID v2 = edge2.to; // u2 -> v2 is an edge in G2

            // Check if this G2 edge was already accounted for (as a substitution/match for a G1 edge)
            // This check depends on whether we consider graphs directed or undirected for edge accounting.
            // If mapped_edges stores canonical form (min,max), then check that.
            // If it stores directed {u2,v2}, check that.
            bool already_accounted = false;
            if (g2_mapped_edges.count({std::min(u2, v2), std::max(u2, v2)}) || g2_mapped_edges.count({u2,v2})) {
                 already_accounted = true;
            }


            if (!already_accounted) {
                // This G2 edge was not a mapping of any G1 edge. So it's an insertion.
                // This also covers cases where u2 or v2 were inserted nodes.
                current_edge_costs += costs_.edge_insertion_cost;
            }
        }
    }
    return current_edge_costs;
}


double HeuristicGED::compute_distance(const Graph& g1, const Graph& g2) {
    double total_ged_cost = 0.0;

    std::map<VertexID, VertexID> mapping_g1_to_g2; // Stores u -> v if u from g1 is mapped to v from g2
    std::set<VertexID> mapped_g2_vertices;

    std::vector<NodeSubstitutionCandidate> candidates;
    for (VertexID u : g1.get_vertices()) {
        for (VertexID v : g2.get_vertices()) {
            candidates.push_back({u, v, get_vertex_substitution_cost(g1, u, g2, v)});
        }
    }
    std::sort(candidates.begin(), candidates.end()); // Sort by cost

    std::set<VertexID> g1_nodes_processed;

    for (const auto& cand : candidates) {
        if (g1_nodes_processed.count(cand.u) || mapped_g2_vertices.count(cand.v)) {
            continue; // u or v already handled
        }

        // Heuristic decision: is substituting cheaper than deleting u and inserting v separately?
        // This condition is a bit simplistic as 'v' might be substituted by another 'u_prime'.
        // A true greedy approach would commit to this substitution if it's the best available.
        // The costs_.vertex_deletion_cost + costs_.vertex_insertion_cost is a threshold.
        // If we don't use this threshold, it becomes a simple best-match greedy assignment.
        // Let's try without the threshold first for a simpler greedy matching.
        // The threshold `costs_.vertex_deletion_cost + costs_.vertex_insertion_cost` is more for algorithms
        // like A* where you decide between mapping u->v vs. deleting u and inserting v.
        // In this greedy pass, we are trying to find the cheapest set of substitutions first.

        // If cost of substituting u with v is less than deleting u and inserting v
        // (assuming v would otherwise be an inserted node if not matched with u)
        // This logic can be tricky. For a simple greedy assignment:
        // If this (u,v) pair is the cheapest available for unmapped u and unmapped v, take it.
        if (cand.cost < (costs_.vertex_deletion_cost + costs_.vertex_insertion_cost) ) { // Heuristic threshold
             mapping_g1_to_g2[cand.u] = cand.v;
             total_ged_cost += cand.cost;
             g1_nodes_processed.insert(cand.u);
             mapped_g2_vertices.insert(cand.v);
        }
        // If we don't use the threshold, it would be:
        // mapping_g1_to_g2[cand.u] = cand.v;
        // total_ged_cost += cand.cost;
        // g1_nodes_processed.insert(cand.u);
        // mapped_g2_vertices.insert(cand.v);

    }

    // Account for deletions (G1 nodes not mapped)
    for (VertexID u : g1.get_vertices()) {
        if (mapping_g1_to_g2.find(u) == mapping_g1_to_g2.end()) {
            total_ged_cost += costs_.vertex_deletion_cost;
        }
    }

    // Account for insertions (G2 nodes not mapped)
    for (VertexID v : g2.get_vertices()) {
        if (mapped_g2_vertices.find(v) == mapped_g2_vertices.end()) {
            total_ged_cost += costs_.vertex_insertion_cost;
        }
    }

    // Calculate edge costs based on the vertex mapping
    total_ged_cost += calculate_edge_costs(g1, g2, mapping_g1_to_g2);

    return total_ged_cost;
}

} // namespace ged
