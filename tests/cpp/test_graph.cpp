#include "gtest/gtest.h"
#include "Graph.h" // Assuming Graph.h is accessible (e.g. via include path in CMake)

// Test fixture for Graph tests
class GraphTest : public ::testing::Test {
protected:
    Graph g;
};

TEST_F(GraphTest, InitialState) {
    EXPECT_EQ(g.num_vertices(), 0);
    EXPECT_EQ(g.num_edges(), 0);
}

TEST_F(GraphTest, AddVertex) {
    ASSERT_TRUE(g.add_vertex(1, "A"));
    EXPECT_EQ(g.num_vertices(), 1);
    EXPECT_TRUE(g.has_vertex(1));
    EXPECT_EQ(g.get_vertex_label(1), "A");

    ASSERT_TRUE(g.add_vertex(2, "B"));
    EXPECT_EQ(g.num_vertices(), 2);
    EXPECT_TRUE(g.has_vertex(2));
    EXPECT_EQ(g.get_vertex_label(2), "B");

    // Try adding existing vertex
    EXPECT_FALSE(g.add_vertex(1, "C"));
    EXPECT_EQ(g.num_vertices(), 2); // Count should not change
    EXPECT_EQ(g.get_vertex_label(1), "A"); // Label should not change
}

TEST_F(GraphTest, RemoveVertex) {
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    g.add_vertex(3, "C");
    g.add_edge(1, 2, "e12");
    g.add_edge(1, 3, "e13");
    g.add_edge(2, 3, "e23");
    ASSERT_EQ(g.num_edges(), 3);

    ASSERT_TRUE(g.remove_vertex(1));
    EXPECT_EQ(g.num_vertices(), 2);
    EXPECT_FALSE(g.has_vertex(1));
    EXPECT_EQ(g.num_edges(), 1); // e12 and e13 should be removed
    EXPECT_FALSE(g.has_edge(1,2));
    EXPECT_FALSE(g.has_edge(1,3));
    EXPECT_TRUE(g.has_edge(2,3));


    // Try removing non-existent vertex
    EXPECT_FALSE(g.remove_vertex(5));
    EXPECT_EQ(g.num_vertices(), 2);
}

TEST_F(GraphTest, VertexLabels) {
    g.add_vertex(1, "Initial");
    EXPECT_EQ(g.get_vertex_label(1), "Initial");
    ASSERT_TRUE(g.set_vertex_label(1, "Updated"));
    EXPECT_EQ(g.get_vertex_label(1), "Updated");

    EXPECT_FALSE(g.set_vertex_label(2, "NonExistent")); // Vertex 2 does not exist
    EXPECT_THROW(g.get_vertex_label(2), std::out_of_range);
}

TEST_F(GraphTest, AddEdge) {
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    g.add_vertex(3, "C");

    ASSERT_TRUE(g.add_edge(1, 2, "e12"));
    EXPECT_EQ(g.num_edges(), 1);
    EXPECT_TRUE(g.has_edge(1, 2));
    EXPECT_EQ(g.get_edge_label(1, 2), "e12");

    // Add another edge
    ASSERT_TRUE(g.add_edge(1, 3, "e13"));
    EXPECT_EQ(g.num_edges(), 2);
    EXPECT_TRUE(g.has_edge(1, 3));

    // Try adding existing edge
    EXPECT_FALSE(g.add_edge(1, 2, "e12_new"));
    EXPECT_EQ(g.num_edges(), 2); // Count should not change
    EXPECT_EQ(g.get_edge_label(1, 2), "e12"); // Label should not change

    // Try adding edge with non-existent vertex
    EXPECT_FALSE(g.add_edge(1, 4, "e14")); // Vertex 4 does not exist
    EXPECT_FALSE(g.add_edge(4, 1, "e41")); // Vertex 4 does not exist
    EXPECT_EQ(g.num_edges(), 2);
}

TEST_F(GraphTest, RemoveEdge) {
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    g.add_edge(1, 2, "e12");
    ASSERT_EQ(g.num_edges(), 1);

    ASSERT_TRUE(g.remove_edge(1, 2));
    EXPECT_EQ(g.num_edges(), 0);
    EXPECT_FALSE(g.has_edge(1, 2));

    // Try removing non-existent edge
    EXPECT_FALSE(g.remove_edge(1, 2)); // Already removed
    g.add_edge(1,2,"e12_again");
    EXPECT_FALSE(g.remove_edge(2, 1)); // Different direction, not added
    EXPECT_EQ(g.num_edges(), 1);
}

TEST_F(GraphTest, EdgeLabels) {
    g.add_vertex(1, "A");
    g.add_vertex(2, "B");
    g.add_edge(1, 2, "InitialEdge");
    EXPECT_EQ(g.get_edge_label(1, 2), "InitialEdge");

    ASSERT_TRUE(g.set_edge_label(1, 2, "UpdatedEdge"));
    EXPECT_EQ(g.get_edge_label(1, 2), "UpdatedEdge");

    EXPECT_FALSE(g.set_edge_label(1, 3, "NonExistent")); // Edge 1-3 does not exist
    EXPECT_THROW(g.get_edge_label(1,3), std::out_of_range);
}

TEST_F(GraphTest, GetVertices) {
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_vertex(3);
    std::set<VertexID> vertices = g.get_vertices();
    std::set<VertexID> expected = {1, 2, 3};
    EXPECT_EQ(vertices, expected);
}

TEST_F(GraphTest, GetEdgesFrom) {
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_vertex(3);
    g.add_edge(1, 2, "L12");
    g.add_edge(1, 3, "L13");

    std::set<Edge> edges_from_1 = g.get_edges_from(1);
    EXPECT_EQ(edges_from_1.size(), 2);
    // Check specific edges - Edge struct needs operator== for find or direct comparison in set
    // Or iterate and check manually
    bool found12 = false, found13 = false;
    for(const auto& edge : edges_from_1) {
        if(edge.to == 2 && edge.label == "L12") found12 = true;
        if(edge.to == 3 && edge.label == "L13") found13 = true;
    }
    EXPECT_TRUE(found12);
    EXPECT_TRUE(found13);

    std::set<Edge> edges_from_2 = g.get_edges_from(2);
    EXPECT_EQ(edges_from_2.size(), 0);
}

TEST_F(GraphTest, GetAllEdges) {
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_vertex(3);
    g.add_edge(1,2, "e12");
    g.add_edge(1,3, "e13");
    g.add_edge(2,3, "e23");

    auto all_edges = g.get_all_edges();
    // Convert to set for easier comparison if order doesn't matter
    std::set<std::pair<VertexID, VertexID>> edge_set(all_edges.begin(), all_edges.end());

    std::set<std::pair<VertexID, VertexID>> expected_edges = {{1,2}, {1,3}, {2,3}};
    EXPECT_EQ(edge_set, expected_edges);
    EXPECT_EQ(all_edges.size(), 3); // Ensure no duplicates if vector is checked
}

TEST_F(GraphTest, PrintGraph) {
    // Just a smoke test to ensure print_graph doesn't crash
    // Actual output checking is more complex and usually not done in unit tests
    g.add_vertex(1, "Node1");
    g.add_edge(1,1, "Loop"); // Self-loop
    ASSERT_NO_THROW(g.print_graph());
}

// Main function for Google Test (needed if not linking gtest_main)
// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
// We link GTest::gtest_main, so this is not strictly needed here.

// Add more tests for remove_vertex specifically for edge cases with incoming edges
TEST_F(GraphTest, RemoveVertexImpactsIncomingEdges) {
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_vertex(3);
    g.add_edge(1,3,"e13"); // Edge to the vertex being removed
    g.add_edge(2,3,"e23"); // Edge to the vertex being removed
    g.add_edge(3,1,"e31"); // Edge from the vertex being removed
    ASSERT_EQ(g.num_edges(), 3);

    g.remove_vertex(3); // Remove vertex 3
    EXPECT_EQ(g.num_vertices(), 2);
    EXPECT_FALSE(g.has_vertex(3));
    EXPECT_EQ(g.num_edges(), 0); // All edges connected to 3 should be gone
    EXPECT_FALSE(g.has_edge(1,3));
    EXPECT_FALSE(g.has_edge(2,3));
    EXPECT_FALSE(g.has_edge(3,1));
}
