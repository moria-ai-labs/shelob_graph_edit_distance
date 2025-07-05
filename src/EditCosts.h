#ifndef EDIT_COSTS_H
#define EDIT_COSTS_H

#include "Graph.h" // For Label type, though not strictly necessary if costs are label-agnostic initially

// Default costs are often 1 for each basic operation.
// Substitution costs can be 0 if labels match, 1 (or other) if they don't.
// For simplicity, we'll start with fixed costs.
// A more advanced version could allow functions or matrices for label-dependent costs.

struct EditCosts {
    double vertex_insertion_cost;
    double vertex_deletion_cost;
    double vertex_substitution_cost; // Cost if labels differ. Assumed 0 if labels match.

    double edge_insertion_cost;
    double edge_deletion_cost;
    // Edge substitution is often modeled as an edge deletion and an edge insertion.
    // Or, it can have its own cost, especially if edges have labels/attributes.
    // For now, let's assume edge substitution is not a primitive, or its cost is high.
    // If we add edge labels to Graph that are used in GED, this might change.
    // double edge_substitution_cost;

    // Default costs, e.g., all operations cost 1.0
    EditCosts(double vi = 1.0, double vd = 1.0, double vs = 1.0,
              double ei = 1.0, double ed = 1.0)
        : vertex_insertion_cost(vi), vertex_deletion_cost(vd),
          vertex_substitution_cost(vs), edge_insertion_cost(ei),
          edge_deletion_cost(ed) {}

    // Placeholder for more complex cost functions if needed later
    // E.g., cost of substituting vertex label l1 with l2
    // double get_vertex_substitution_cost(const Label& l1, const Label& l2) const {
    //     return (l1 == l2) ? 0.0 : vertex_substitution_cost;
    // }
};

#endif // EDIT_COSTS_H
