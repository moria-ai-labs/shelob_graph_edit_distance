#ifndef GRAPH_ALGOS_H
#define GRAPH_ALGOS_H

#include "Graph.h"
#include <vector>
#include <map>

// Forward declare Eigen types if possible to reduce header compile times for users of GraphAlgos.h
// However, for implementation, we'll need the full Eigen headers in .cpp
// For simplicity here, and since Eigen is header-only, direct include is also fine.
// #include <Eigen/Dense>
// #include <Eigen/Sparse>
// #include <Eigen/Eigenvalues>

namespace ged {

// Using an alias for Eigen's MatrixXd for convenience.
// This will be properly defined when Eigen headers are included in .cpp or here.
// For now, this is conceptual.
// using Matrix = Eigen::MatrixXd;
// using Vector = Eigen::VectorXd;

class SpectralEmbedder {
public:
    SpectralEmbedder(int k = 5); // k: number of eigenvalues to use for embedding

    // Computes the Adjacency Matrix of a graph
    // Returns a dense matrix. For very large sparse graphs, Eigen::SparseMatrix would be better.
    // We need a consistent mapping from VertexID to matrix row/col index.
    static std::vector<std::vector<double>> get_adjacency_matrix(const Graph& g, const std::vector<VertexID>& node_order_map);

    // Computes the Laplacian Matrix of a graph: L = D - A
    static std::vector<std::vector<double>> get_laplacian_matrix(const Graph& g, const std::vector<VertexID>& node_order_map);

    // Generates an embedding for the graph using the smallest k eigenvalues of its Laplacian
    std::vector<double> embed_graph(const Graph& g);

    // Computes the Euclidean distance between two graph embeddings
    static double embedding_distance(const std::vector<double>& emb1, const std::vector<double>& emb2);

public: // Made public static for easier binding and utility
    // Helper to get a consistent node ordering for matrix construction
    static std::vector<VertexID> get_node_ordering_public(const Graph& g);

private:
    int num_eigenvalues_k_;
};

} // namespace ged

#endif // GRAPH_ALGOS_H
