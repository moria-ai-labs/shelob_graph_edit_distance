#include "gtest/gtest.h"
#include "Graph.h"
#include "GraphAlgos.h" // For SpectralEmbedder
#include <vector>
#include <cmath> // For std::abs, std::sqrt
#include <numeric> // For std::iota for node order map generation (if needed)
#include <algorithm> // For std::sort

// Helper function to compare vectors of doubles with a tolerance
void ExpectVectorsNear(const std::vector<double>& v1, const std::vector<double>& v2, double tolerance) {
    ASSERT_EQ(v1.size(), v2.size());
    for (size_t i = 0; i < v1.size(); ++i) {
        EXPECT_NEAR(v1[i], v2[i], tolerance);
    }
}

// Helper function to compare matrices (vector of vectors)
void ExpectMatricesEqual(const std::vector<std::vector<double>>& m1, const std::vector<std::vector<double>>& m2) {
    ASSERT_EQ(m1.size(), m2.size());
    for (size_t i = 0; i < m1.size(); ++i) {
        ASSERT_EQ(m1[i].size(), m2[i].size());
        for (size_t j = 0; j < m1[i].size(); ++j) {
            EXPECT_DOUBLE_EQ(m1[i][j], m2[i][j]);
        }
    }
}


class SpectralEmbedderTest : public ::testing::Test {
protected:
    // ged::SpectralEmbedder embedder; // Default k=5
    // SpectralEmbedderTest() : embedder(3) {} // Example: k=3

    Graph create_path_graph(int num_nodes) {
        Graph g;
        if (num_nodes <= 0) return g;
        for (int i = 1; i <= num_nodes; ++i) {
            g.add_vertex(i, "v" + std::to_string(i));
        }
        for (int i = 1; i < num_nodes; ++i) {
            g.add_edge(i, i + 1, "e"); // Directed, but Laplacian uses undirected version
        }
        return g;
    }

    Graph create_cycle_graph(int num_nodes) {
        Graph g;
        if (num_nodes <=0) return g;
        for(int i=1; i<=num_nodes; ++i) g.add_vertex(i, "v"+std::to_string(i));
        for(int i=1; i<num_nodes; ++i) g.add_edge(i, i+1, "e");
        if (num_nodes > 1) g.add_edge(num_nodes, 1, "e"); // Close the cycle
        return g;
    }
};

TEST_F(SpectralEmbedderTest, GetNodeOrdering) {
    Graph g;
    g.add_vertex(3, "C");
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    std::vector<VertexID> expected_order = {1, 2, 3};
    std::vector<VertexID> actual_order = ged::SpectralEmbedder::get_node_ordering_public(g);
    EXPECT_EQ(actual_order, expected_order);

    Graph empty_g;
    EXPECT_TRUE(ged::SpectralEmbedder::get_node_ordering_public(empty_g).empty());
}

TEST_F(SpectralEmbedderTest, AdjacencyMatrix) {
    Graph g; // 1 -> 2
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    g.add_edge(1, 2, "e12");

    std::vector<VertexID> node_order = {1, 2}; // Manual order for testing
    std::vector<std::vector<double>> adj = ged::SpectralEmbedder::get_adjacency_matrix(g, node_order);
    std::vector<std::vector<double>> expected_adj = {{0.0, 1.0}, {0.0, 0.0}};
    ExpectMatricesEqual(adj, expected_adj);

    Graph g_undir; // 1 -- 2 (add both directions for test)
    g_undir.add_vertex(1); g_undir.add_vertex(2);
    g_undir.add_edge(1,2); g_undir.add_edge(2,1);
    // Note: get_adjacency_matrix as implemented uses the directed edges from Graph.
    // The Laplacian calculation internally creates a symmetric version for A.
    adj = ged::SpectralEmbedder::get_adjacency_matrix(g_undir, node_order);
    expected_adj = {{0.0, 1.0}, {1.0, 0.0}};
    ExpectMatricesEqual(adj, expected_adj);
}

TEST_F(SpectralEmbedderTest, LaplacianMatrix_P2) { // Path graph P2: 1-2
    Graph p2 = create_path_graph(2); // 1 -- 2 (effectively, for Laplacian)
    std::vector<VertexID> node_order = ged::SpectralEmbedder::get_node_ordering_public(p2); // Should be {1,2}

    std::vector<std::vector<double>> laplacian = ged::SpectralEmbedder::get_laplacian_matrix(p2, node_order);
    // L = D - A
    // A_undir for 1-2: [[0,1],[1,0]]
    // D for 1-2: [[1,0],[0,1]]
    // L = [[1,-1],[-1,1]]
    std::vector<std::vector<double>> expected_laplacian = {{1.0, -1.0}, {-1.0, 1.0}};
    ExpectMatricesEqual(laplacian, expected_laplacian);
}

TEST_F(SpectralEmbedderTest, LaplacianMatrix_P3) { // Path graph P3: 1-2-3
    Graph p3 = create_path_graph(3);
    std::vector<VertexID> node_order = ged::SpectralEmbedder::get_node_ordering_public(p3); // {1,2,3}

    // A_undir for 1-2-3:
    //   1 2 3
    // 1[0,1,0]
    // 2[1,0,1]
    // 3[0,1,0]
    // D for 1-2-3:
    //   1 2 3
    // 1[1,0,0]
    // 2[0,2,0]
    // 3[0,0,1]
    // L = D - A_undir:
    //   1  2  3
    // 1[1,-1, 0]
    // 2[-1,2,-1]
    // 3[0,-1, 1]
    std::vector<std::vector<double>> laplacian = ged::SpectralEmbedder::get_laplacian_matrix(p3, node_order);
    std::vector<std::vector<double>> expected_laplacian = {
        {1.0, -1.0, 0.0},
        {-1.0, 2.0, -1.0},
        {0.0, -1.0, 1.0}
    };
    ExpectMatricesEqual(laplacian, expected_laplacian);
}


TEST_F(SpectralEmbedderTest, EmbedGraph_Empty) {
    ged::SpectralEmbedder embedder(3);
    Graph g;
    std::vector<double> emb = embedder.embed_graph(g);
    ExpectVectorsNear(emb, {0.0, 0.0, 0.0}, 1e-9);
}

TEST_F(SpectralEmbedderTest, EmbedGraph_P2_k2) { // Path graph P2: 1-2
    ged::SpectralEmbedder embedder(2); // k=2
    Graph p2 = create_path_graph(2);
    // L = [[1,-1],[-1,1]]. Eigenvalues are 0, 2.
    std::vector<double> emb = embedder.embed_graph(p2);
    ExpectVectorsNear(emb, {0.0, 2.0}, 1e-9);
}

TEST_F(SpectralEmbedderTest, EmbedGraph_P3_k3) { // Path graph P3: 1-2-3
    ged::SpectralEmbedder embedder(3); // k=3
    Graph p3 = create_path_graph(3);
    // L = [[1,-1,0],[-1,2,-1],[0,-1,1]]. Eigenvalues are 0, 1, 3.
    std::vector<double> emb = embedder.embed_graph(p3);
    ExpectVectorsNear(emb, {0.0, 1.0, 3.0}, 1e-9);
}

TEST_F(SpectralEmbedderTest, EmbedGraph_C3_k3) { // Cycle graph C3 (triangle)
    ged::SpectralEmbedder embedder(3); // k=3
    Graph c3 = create_cycle_graph(3);
    // L for C3:
    // A_undir: [[0,1,1],[1,0,1],[1,1,0]]
    // D: [[2,0,0],[0,2,0],[0,0,2]]
    // L = [[2,-1,-1],[-1,2,-1],[-1,-1,2]]
    // Eigenvalues are 0, 3, 3.
    std::vector<double> emb = embedder.embed_graph(c3);
    ExpectVectorsNear(emb, {0.0, 3.0, 3.0}, 1e-9);
}

TEST_F(SpectralEmbedderTest, EmbedGraph_Disconnected_2P2_k4) { // Two P2 graphs (1-2, 3-4)
    ged::SpectralEmbedder embedder(4); // k=4
    Graph g;
    g.add_vertex(1); g.add_vertex(2); g.add_vertex(3); g.add_vertex(4);
    g.add_edge(1,2); // Component 1: 1-2
    g.add_edge(3,4); // Component 2: 3-4
    // Laplacian is block diagonal of two P2 Laplacians.
    // Eigenvalues for P2: 0, 2.
    // For 2P2: 0, 0, 2, 2.
    std::vector<double> emb = embedder.embed_graph(g);
    ExpectVectorsNear(emb, {0.0, 0.0, 2.0, 2.0}, 1e-9);
}


TEST_F(SpectralEmbedderTest, EmbedGraph_InsufficientVerticesForK) {
    ged::SpectralEmbedder embedder(5); // k=5
    Graph p2 = create_path_graph(2); // Only 2 vertices, so at most 2 eigenvalues.
    // L = [[1,-1],[-1,1]]. Eigenvalues are 0, 2.
    // Expected embedding: {0.0, 2.0, 0.0, 0.0, 0.0} (padded with zeros)
    std::vector<double> emb = embedder.embed_graph(p2);
    ExpectVectorsNear(emb, {0.0, 2.0, 0.0, 0.0, 0.0}, 1e-9);
}


TEST_F(SpectralEmbedderTest, EmbeddingDistance) {
    std::vector<double> emb1 = {0.0, 1.0, 2.0};
    std::vector<double> emb2 = {0.0, 1.0, 2.0}; // Identical
    std::vector<double> emb3 = {1.0, 1.0, 3.0};

    EXPECT_DOUBLE_EQ(ged::SpectralEmbedder::embedding_distance(emb1, emb2), 0.0);

    // dist(emb1, emb3) = sqrt((0-1)^2 + (1-1)^2 + (2-3)^2) = sqrt(1 + 0 + 1) = sqrt(2)
    EXPECT_NEAR(ged::SpectralEmbedder::embedding_distance(emb1, emb3), std::sqrt(2.0), 1e-9);

    std::vector<double> empty_emb1, empty_emb2;
    EXPECT_DOUBLE_EQ(ged::SpectralEmbedder::embedding_distance(empty_emb1, empty_emb2), 0.0);

    EXPECT_THROW(ged::SpectralEmbedder::embedding_distance(emb1, {1.0}), std::invalid_argument);
}

TEST_F(SpectralEmbedderTest, ConstructorInvalidK) {
    EXPECT_THROW(ged::SpectralEmbedder embedder(0), std::invalid_argument);
    EXPECT_THROW(ged::SpectralEmbedder embedder(-1), std::invalid_argument);
}
