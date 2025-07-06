#include "gtest/gtest.h"
#include "Graph.h"
#include "GEDAlgorithm.h"
#include "EditCosts.h"

// Test fixture for GEDAlgorithm tests
class GEDAlgorithmTest : public ::testing::Test {
protected:
    EditCosts costs; // Default costs: 1.0 for all operations
    ged::HeuristicGED ged_calculator;

    GEDAlgorithmTest() : ged_calculator(costs) {}

    Graph create_simple_graph1() {
        Graph g;
        g.add_vertex(1, "A");
        g.add_vertex(2, "B");
        g.add_edge(1, 2, "e12");
        return g;
    }

    Graph create_simple_graph2() {
        Graph g;
        g.add_vertex(10, "A"); // Same label as g1's node 1
        g.add_vertex(20, "C"); // Different label from g1's node 2
        g.add_edge(10, 20, "e1020");
        return g;
    }
};

TEST_F(GEDAlgorithmTest, EmptyGraphs) {
    Graph g1, g2;
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), 0.0);
}

TEST_F(GEDAlgorithmTest, OneEmptyGraph) {
    Graph g1 = create_simple_graph1(); // 2 vertices, 1 edge
    Graph g2;
    // Expected: delete 2 vertices (2*1.0) + delete 1 edge (1*1.0) = 3.0
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), 3.0);
    // Expected: insert 2 vertices (2*1.0) + insert 1 edge (1*1.0) = 3.0
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g2, g1), 3.0);
}

TEST_F(GEDAlgorithmTest, IdenticalGraphs) {
    Graph g1 = create_simple_graph1();
    Graph g1_copy; // Build identical graph with different node IDs but same structure/labels
    g1_copy.add_vertex(1, "A");
    g1_copy.add_vertex(2, "B");
    g1_copy.add_edge(1, 2, "e12");

    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g1_copy), 0.0);
}

TEST_F(GEDAlgorithmTest, SingleVertexSubstitution) {
    Graph g1;
    g1.add_vertex(1, "A");
    Graph g2;
    g2.add_vertex(10, "B"); // Different label

    // Expected: substitute vertex 1 with 10 (cost 1.0)
    // The greedy algorithm should map 1->10 with cost 1.0 (substitution)
    // vs. delete 1 (cost 1) + insert 10 (cost 1) = 2
    // So substitution is chosen.
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), costs.vertex_substitution_cost);
}

TEST_F(GEDAlgorithmTest, SingleVertexDeletion) {
    Graph g1;
    g1.add_vertex(1, "A");
    Graph g2; // Empty
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), costs.vertex_deletion_cost);
}

TEST_F(GEDAlgorithmTest, SingleVertexInsertion) {
    Graph g1; // Empty
    Graph g2;
    g2.add_vertex(1, "A");
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), costs.vertex_insertion_cost);
}

TEST_F(GEDAlgorithmTest, SimpleGraphTransformation1) {
    // G1: 1(A) -> 2(B)
    // G2: 10(A) -> 20(C)
    // Edges: (1,2) in G1, (10,20) in G2
    Graph g1 = create_simple_graph1();
    Graph g2 = create_simple_graph2();

    // Expected heuristic mapping:
    // 1(A) -> 10(A) : cost 0 (labels match)
    // 2(B) -> 20(C) : cost 1 (labels differ, substitution)
    // Total vertex cost = 1.0
    // Edge (1,2) in G1 maps to (10,20) in G2.
    // Edge (10,20) exists in G2. So edge cost is 0.
    // Total GED = 1.0
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), 1.0);
}


TEST_F(GEDAlgorithmTest, GraphWithMoreDeletionsAndInsertions) {
    Graph g1;
    g1.add_vertex(1, "A");
    g1.add_vertex(2, "B");
    g1.add_vertex(3, "X"); // This node will be deleted
    g1.add_edge(1,2, "e12");
    g1.add_edge(1,3, "e1X"); // This edge will be deleted

    Graph g2;
    g2.add_vertex(10, "A");
    g2.add_vertex(20, "B");
    g2.add_vertex(40, "Y"); // This node will be inserted
    g2.add_edge(10,20, "e1020");
    g2.add_edge(10,40, "e10Y"); // This edge will be inserted

    // With the threshold `cand.cost < (costs_.vertex_deletion_cost + costs_.vertex_insertion_cost)`:
    // (1,10,0): 0 < 2. Map 1->10. Cost=0.
    // (2,20,0): 0 < 2. Map 2->20. Cost=0.
    // (3,40,1): 1 < 2. Map 3->40. Cost=1. (Labels X and Y differ)
    // Total vertex substitution cost = 1.
    // No explicit deletions or insertions of vertices according to the greedy mapping logic.
    // Edge costs with mapping {1:10, 2:20, 3:40}:
    // G1 (1,2) -> G2 (10,20). Exists. Cost 0.
    // G1 (1,3) -> G2 (10,40). Exists. Cost 0.
    // All G2 edges are covered by these mappings.
    // Total edge cost = 0.
    // Final GED = 1.0 (vertex substitution) + 0.0 (edge) = 1.0.
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1, g2), 1.0);
}

TEST_F(GEDAlgorithmTest, CustomCosts) {
    EditCosts custom_costs(
        /*vertex_insertion*/ 5.0,
        /*vertex_deletion*/ 4.0,
        /*vertex_substitution*/ 3.0,
        /*edge_insertion*/ 2.0,
        /*edge_deletion*/ 1.0
    );
    ged::HeuristicGED custom_ged_calculator(custom_costs);

    Graph g1; g1.add_vertex(1, "A");
    Graph g2; g2.add_vertex(10, "B"); // Substitute

    // Cost of sub(A,B) = 3.0. Cost of del(A)+ins(B) = 4.0 + 5.0 = 9.0.
    // 3.0 < 9.0, so substitution is chosen.
    EXPECT_DOUBLE_EQ(custom_ged_calculator.compute_distance(g1, g2), 3.0);


    Graph g3; g3.add_vertex(1, "A"); g3.add_vertex(2, "B"); g3.add_edge(1,2);
    Graph g4; // empty
    // Delete 2 vertices (2*4.0=8.0), delete 1 edge (1.0) = 9.0
    EXPECT_DOUBLE_EQ(custom_ged_calculator.compute_distance(g3, g4), 9.0);
}

TEST_F(GEDAlgorithmTest, NoGoodSubstitutions) {
    Graph g1; g1.add_vertex(1, "A");
    Graph g2; g2.add_vertex(2, "B");

    // Costs: vi=1, vd=1, vs=10 (high substitution cost)
    EditCosts high_sub_cost(1.0, 1.0, 10.0, 1.0, 1.0);
    ged::HeuristicGED calc(high_sub_cost);

    // Greedily, sub(A,B) costs 10. del(A)+ins(B) costs 1+1=2.
    // Since 10 is not < 2, the substitution (1,2) is not chosen by the greedy algorithm.
    // So, 1 is deleted (cost 1), 2 is inserted (cost 1). Total = 2.
    EXPECT_DOUBLE_EQ(calc.compute_distance(g1, g2), 2.0);
}

TEST_F(GEDAlgorithmTest, EdgeOnlyChange) {
    Graph g1;
    g1.add_vertex(1,"A"); g1.add_vertex(2,"B");
    g1.add_edge(1,2,"e12");

    Graph g2;
    g2.add_vertex(1,"A"); g2.add_vertex(2,"B");
    // No edge in g2

    // Mapping: 1->1 (cost 0), 2->2 (cost 0). Vertex cost = 0.
    // Edge: G1 has (1,2). Mapped G2 does not have (1,2). So delete edge (1,2) -> cost 1.0.
    // No G2 edges to insert.
    // Total = 1.0
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g1,g2), costs.edge_deletion_cost);

    // Reverse: G1 no edge, G2 has edge
    // Total = 1.0 (edge insertion)
    EXPECT_DOUBLE_EQ(ged_calculator.compute_distance(g2,g1), costs.edge_insertion_cost);
}
