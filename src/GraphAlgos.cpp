#include "GraphAlgos.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues> // For SelfAdjointEigenSolver
#include <algorithm>    // For std::sort, std::iota
#include <stdexcept>    // For std::runtime_error
#include <vector>
#include <map>

namespace ged {

// Helper to create a mapping from VertexID to a 0-based index
// and a reverse mapping from index to VertexID (which is just the vector itself)
std::vector<VertexID> SpectralEmbedder::get_node_ordering_public(const Graph& g) {
    std::vector<VertexID> node_list;
    for (VertexID v_id : g.get_vertices()) {
        node_list.push_back(v_id);
    }
    std::sort(node_list.begin(), node_list.end()); // Ensure consistent ordering
    return node_list;
}


std::vector<std::vector<double>> SpectralEmbedder::get_adjacency_matrix(const Graph& g, const std::vector<VertexID>& node_order_param) {
    // Ensure consistency if node_order_param is used, or always derive it if that's safer.
    // For now, assume node_order_param is the one to use if provided and valid.
    // If node_order_param is empty but g is not, it's an issue.
    // The public static version called from bindings will use get_node_ordering_public directly.
    // This internal one might be called by other C++ code that already has an order.
    const std::vector<VertexID>& node_order = node_order_param; // Use the provided one.
    size_t n = node_order.size();
    if (n == 0 && g.num_vertices() == 0) return {}; // Empty graph, empty matrix
    if (n == 0 && g.num_vertices() > 0) {
        throw std::runtime_error("Adjacency matrix: node_order is empty for a non-empty graph.");
    }
    if (n != g.num_vertices()) {
        // This check is tricky if g can have isolated vertices not in node_order,
        // or if node_order contains external IDs.
        // Assuming node_order is a permutation of g.get_vertices() or a subset.
        // For simplicity, let's assume node_order is derived from g consistently.
    }

    std::map<VertexID, size_t> node_to_idx;
    for (size_t i = 0; i < n; ++i) {
        node_to_idx[node_order[i]] = i;
    }

    std::vector<std::vector<double>> adj_matrix(n, std::vector<double>(n, 0.0));

    for (VertexID u_id : node_order) {
        if (!g.has_vertex(u_id)) continue; // Should not happen if node_order is from g.get_vertices()
        size_t u_idx = node_to_idx[u_id];
        for (const auto& edge : g.get_edges_from(u_id)) {
            VertexID v_id = edge.to;
            if (node_to_idx.count(v_id)) { // Ensure target vertex is in the considered set
                size_t v_idx = node_to_idx[v_id];
                adj_matrix[u_idx][v_idx] = 1.0; // Assuming unweighted graph for adjacency
                // If undirected, adj_matrix[v_idx][u_idx] = 1.0;
                // Graph class is directed, so this is fine.
                // For undirected Laplacian, often A is made symmetric.
            }
        }
    }
    return adj_matrix;
}


std::vector<std::vector<double>> SpectralEmbedder::get_laplacian_matrix(const Graph& g, const std::vector<VertexID>& node_order) {
    size_t n = node_order.size();
    if (n == 0) return {};

    std::map<VertexID, size_t> node_to_idx;
    for (size_t i = 0; i < n; ++i) {
        node_to_idx[node_order[i]] = i;
    }

    // For Laplacian of a directed graph, interpretations vary.
    // Often, GED and spectral methods assume undirected graphs.
    // Let's construct Adjacency for an *undirected version* of g for the Laplacian.
    Eigen::MatrixXd A_undirected = Eigen::MatrixXd::Zero(n, n);
    for (VertexID u_id : node_order) {
        size_t u_idx = node_to_idx[u_id];
        for (const auto& edge : g.get_edges_from(u_id)) {
            if (node_to_idx.count(edge.to)) {
                size_t v_idx = node_to_idx[edge.to];
                A_undirected(u_idx, v_idx) = 1.0;
                A_undirected(v_idx, u_idx) = 1.0; // Make it symmetric for standard Laplacian
            }
        }
    }

    Eigen::MatrixXd D = Eigen::MatrixXd::Zero(n, n);
    for (size_t i = 0; i < n; ++i) {
        D(i, i) = A_undirected.row(i).sum(); // Degree = sum of row in symmetric adjacency
    }

    Eigen::MatrixXd L_eigen = D - A_undirected;

    // Convert Eigen::MatrixXd to std::vector<std::vector<double>>
    std::vector<std::vector<double>> laplacian_matrix(n, std::vector<double>(n));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            laplacian_matrix[i][j] = L_eigen(i, j);
        }
    }
    return laplacian_matrix;
}


SpectralEmbedder::SpectralEmbedder(int k) : num_eigenvalues_k_(k) {
    if (k <= 0) {
        throw std::invalid_argument("Number of eigenvalues (k) must be positive.");
    }
}

std::vector<double> SpectralEmbedder::embed_graph(const Graph& g) {
    if (g.num_vertices() == 0) {
        // Return a vector of k zeros, or handle as an error/special case
        return std::vector<double>(num_eigenvalues_k_, 0.0);
    }

    std::vector<VertexID> node_order = get_node_ordering(g);
    size_t n = node_order.size();

    // Recalculate Adjacency and Degree matrices using Eigen directly for eigenvalue computation
    Eigen::MatrixXd A_undir = Eigen::MatrixXd::Zero(n, n);
    std::map<VertexID, size_t> node_to_idx;
    for(size_t i=0; i<n; ++i) node_to_idx[node_order[i]] = i;

    for (VertexID u_id : node_order) {
        size_t u_idx = node_to_idx[u_id];
        for (const auto& edge : g.get_edges_from(u_id)) {
            if (node_to_idx.count(edge.to)) {
                size_t v_idx = node_to_idx[edge.to];
                A_undir(u_idx, v_idx) = 1.0;
                A_undir(v_idx, u_idx) = 1.0; // Symmetric for standard Laplacian
            }
        }
    }
    Eigen::MatrixXd D_diag = Eigen::MatrixXd::Zero(n,n);
    for(size_t i=0; i<n; ++i) D_diag(i,i) = A_undir.row(i).sum();

    Eigen::MatrixXd L_eigen = D_diag - A_undir;

    if (n == 0) { // Should be caught by g.num_vertices() == 0 earlier
        return std::vector<double>(num_eigenvalues_k_, 0.0);
    }

    // Compute eigenvalues. Laplacian is symmetric, so use SelfAdjointEigenSolver.
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(L_eigen);
    if (eigensolver.info() != Eigen::Success) {
        throw std::runtime_error("Eigenvalue computation failed.");
    }

    Eigen::VectorXd eigenvalues_eigen = eigensolver.eigenvalues(); // Sorted in increasing order

    std::vector<double> embedding;
    // Take the smallest 'k' eigenvalues. If fewer than 'k' available, pad with a value (e.g., 0 or a large number).
    // It's typical to take the smallest non-trivial eigenvalues. The smallest is often 0.
    for (int i = 0; i < num_eigenvalues_k_; ++i) {
        if (i < eigenvalues_eigen.size()) {
            embedding.push_back(eigenvalues_eigen(i));
        } else {
            // Pad if graph has fewer than k eigenvalues (i.e., n < k)
            // Padding value can be debated; 0.0 is simple.
            // Or use a value that indicates "missing", e.g. largest possible double, if comparison logic handles it.
            embedding.push_back(0.0);
        }
    }
    return embedding;
}

double SpectralEmbedder::embedding_distance(const std::vector<double>& emb1, const std::vector<double>& emb2) {
    if (emb1.size() != emb2.size()) {
        throw std::invalid_argument("Embeddings must have the same dimension for distance calculation.");
    }
    if (emb1.empty()) { // And emb2 is also empty
        return 0.0;
    }

    double sum_sq_diff = 0.0;
    for (size_t i = 0; i < emb1.size(); ++i) {
        double diff = emb1[i] - emb2[i];
        sum_sq_diff += diff * diff;
    }
    return std::sqrt(sum_sq_diff);
}

} // namespace ged
