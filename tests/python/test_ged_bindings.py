import pytest
# Adjust import path if your setup.py puts the module elsewhere or if using a src layout
# For now, assume 'graph_edit_distance' is directly importable after `pip install -e .`
try:
    import graph_edit_distance
    from graph_edit_distance import Graph, Edge, EditCosts, HeuristicGED
except ImportError:
    # This might happen if the C++ module isn't built or not in PYTHONPATH
    # For testing in some environments, you might need to adjust sys.path
    # or ensure the build happens before tests run.
    # For now, we assume it's importable.
    pytest.skip("graph_edit_distance module not found, skipping binding tests. Ensure it's built and installed.", allow_module_level=True)


@pytest.fixture
def simple_graph1():
    g = Graph()
    g.add_vertex(1, "A")
    g.add_vertex(2, "B")
    g.add_edge(1, 2, "e12")
    return g

@pytest.fixture
def simple_graph2():
    g = Graph()
    g.add_vertex(10, "A") # Same label as g1's node 1
    g.add_vertex(20, "C") # Different label from g1's node 2
    g.add_edge(10, 20, "e1020")
    return g

@pytest.fixture
def default_costs():
    return EditCosts()

@pytest.fixture
def ged_calculator(default_costs):
    return HeuristicGED(default_costs)

# --- Graph Class Tests (Python bindings) ---
def test_graph_creation_py():
    g = Graph()
    assert g.num_vertices() == 0
    assert g.num_edges() == 0
    assert len(g) == 0 # Test __len__

def test_graph_add_vertex_py(simple_graph1): # simple_graph1 fixture already adds vertices
    g = simple_graph1
    assert g.num_vertices() == 2
    assert g.has_vertex(1)
    assert g.has_vertex(2)
    assert not g.has_vertex(3)
    assert g.get_vertex_label(1) == "A"
    assert (1 in g) # Test __contains__
    assert (3 not in g)

    # Try adding existing
    assert not g.add_vertex(1, "NewA")
    assert g.get_vertex_label(1) == "A" # Label should not change

def test_graph_remove_vertex_py():
    g = Graph()
    g.add_vertex(1,"A"); g.add_vertex(2,"B"); g.add_edge(1,2,"e12")
    assert g.remove_vertex(1)
    assert not g.has_vertex(1)
    assert g.num_vertices() == 1
    assert g.num_edges() == 0 # Edge (1,2) should be removed
    assert not g.has_edge(1,2)

def test_graph_add_edge_py(simple_graph1):
    g = simple_graph1 # Already has edge (1,2)
    assert g.has_edge(1,2)
    assert g.get_edge_label(1,2) == "e12"

    g.add_vertex(3, "C")
    assert g.add_edge(1,3, "e13")
    assert g.num_edges() == 2
    assert g.has_edge(1,3)

    # Try adding existing
    assert not g.add_edge(1,2, "new_e12")
    assert g.get_edge_label(1,2) == "e12"

def test_graph_remove_edge_py(simple_graph1):
    g = simple_graph1
    assert g.remove_edge(1,2)
    assert not g.has_edge(1,2)
    assert g.num_edges() == 0

def test_graph_get_vertices_py(simple_graph1):
    assert set(simple_graph1.get_vertices()) == {1,2}

def test_graph_get_edges_from_py(simple_graph1):
    edges = simple_graph1.get_edges_from(1)
    assert len(edges) == 1
    # Convert set to list to access element; assumes single edge for simplicity
    edge_list = list(edges)
    edge = edge_list[0]
    assert edge.to == 2
    assert edge.label == "e12"
    assert repr(edge) == "<Edge to=2 label='e12'>"


def test_graph_get_all_edges_py(simple_graph1):
    all_edges = simple_graph1.get_all_edges()
    assert len(all_edges) == 1
    # all_edges is a list of pairs. Access first element, then convert to tuple if needed for comparison.
    # The C++ returns std::vector<std::pair<VertexID, VertexID>>, pybind11 converts to list of tuples/lists.
    # Let's ensure it's a list of (typically) tuples.
    assert isinstance(all_edges, list)
    assert isinstance(all_edges[0], tuple) # pybind11 usually converts std::pair to tuple
    assert all_edges[0] == (1,2)


def test_graph_str_repr_py(simple_graph1):
    assert str(simple_graph1) == "<Graph with 2 vertices and 1 edges>"
    # repr might be the same or different, pybind11 default is usually informative
    # print(repr(simple_graph1))


# --- EditCosts Class Tests (Python bindings) ---
def test_edit_costs_creation_py():
    costs = EditCosts() # Default
    assert costs.vertex_insertion_cost == 1.0
    assert costs.vertex_deletion_cost == 1.0
    assert costs.vertex_substitution_cost == 1.0
    assert costs.edge_insertion_cost == 1.0
    assert costs.edge_deletion_cost == 1.0

    custom_costs = EditCosts(vertex_insertion_cost=2.0, vertex_deletion_cost=2.0, vertex_substitution_cost=3.0, edge_insertion_cost=0.5, edge_deletion_cost=0.5)
    assert custom_costs.vertex_substitution_cost == 3.0
    assert custom_costs.edge_insertion_cost == 0.5

    assert "vi=2.0" in repr(custom_costs) # Check if floats are formatted as expected, may vary

# --- HeuristicGED Class Tests (Python bindings) ---
def test_heuristic_ged_empty_graphs_py(ged_calculator):
    g1 = Graph()
    g2 = Graph()
    assert ged_calculator.compute_distance(g1, g2) == 0.0

def test_heuristic_ged_one_empty_graph_py(ged_calculator, simple_graph1, default_costs):
    g2 = Graph()
    # simple_graph1 has 2 vertices, 1 edge. Costs are all 1.0.
    # Expected: del 2V (2*1) + del 1E (1*1) = 3.0
    assert ged_calculator.compute_distance(simple_graph1, g2) == 3.0
    assert ged_calculator.compute_distance(g2, simple_graph1) == 3.0

def test_heuristic_ged_identical_graphs_py(ged_calculator, simple_graph1):
    g1_copy = Graph()
    g1_copy.add_vertex(1, "A")
    g1_copy.add_vertex(2, "B")
    g1_copy.add_edge(1, 2, "e12")
    assert ged_calculator.compute_distance(simple_graph1, g1_copy) == 0.0

def test_heuristic_ged_simple_substitution_py(ged_calculator, default_costs):
    g1 = Graph(); g1.add_vertex(1, "A")
    g2 = Graph(); g2.add_vertex(10, "B")
    # Expected: substitute 1(A) with 10(B), cost = 1.0 (vertex_substitution_cost)
    assert ged_calculator.compute_distance(g1, g2) == default_costs.vertex_substitution_cost

def test_heuristic_ged_transformation1_py(ged_calculator, simple_graph1, simple_graph2):
    # G1: 1(A) -> 2(B)
    # G2: 10(A) -> 20(C)
    # Expected cost: 1.0 (substitute 2(B) with 20(C))
    # Mapping: 1->10 (cost 0), 2->20 (cost 1). Edge (1,2) maps to (10,20), exists. Total 1.0.
    assert ged_calculator.compute_distance(simple_graph1, simple_graph2) == 1.0

def test_heuristic_ged_custom_costs_py():
    custom_costs = EditCosts(vertex_insertion_cost=5.0, vertex_deletion_cost=4.0, vertex_substitution_cost=3.0, edge_insertion_cost=2.0, edge_deletion_cost=1.0)
    calc = HeuristicGED(custom_costs)

    g1 = Graph(); g1.add_vertex(1, "A")
    g2 = Graph(); g2.add_vertex(10, "B")
    # Cost sub(A,B) = 3. Cost del(A)+ins(B) = 4+5=9.  3 < 9. So sub chosen.
    assert calc.compute_distance(g1, g2) == 3.0

    g3 = Graph(); g3.add_vertex(1,"X"); g3.add_vertex(2,"Y"); g3.add_edge(1,2)
    g4 = Graph() # empty
    # Delete 2V (2*4=8), delete 1E (1*1=1). Total 9.0
    assert calc.compute_distance(g3, g4) == 9.0

def test_heuristic_ged_no_good_substitutions_py():
    # vs=10, all other costs=1
    costs = EditCosts(vertex_insertion_cost=1.0, vertex_deletion_cost=1.0, vertex_substitution_cost=10.0,
                      edge_insertion_cost=1.0, edge_deletion_cost=1.0)
    calc = HeuristicGED(costs)
    g1 = Graph(); g1.add_vertex(1, "A")
    g2 = Graph(); g2.add_vertex(2, "B")
    # Sub(A,B) costs 10. Del(A)+Ins(B) costs 1+1=2.
    # Greedy choice: 10 is not < 2. So 1 is deleted (cost 1), 2 is inserted (cost 1). Total 2.
    assert calc.compute_distance(g1, g2) == 2.0

def test_module_version_py():
    # Checks if __version__ is exposed (optional, but good practice)
    assert hasattr(graph_edit_distance, "__version__")
    assert isinstance(graph_edit_distance.__version__, str)
    # Could also check graph_edit_distance.graph_edit_distance_cpp.__version__
    assert hasattr(graph_edit_distance.graph_edit_distance_cpp, "__version__")

# --- Add SpectralEmbedder tests below this line ---

@pytest.fixture
def spectral_embedder_k3():
    return graph_edit_distance.SpectralEmbedder(k=3)

@pytest.fixture
def spectral_embedder_k5():
    return graph_edit_distance.SpectralEmbedder(k=5)

def expect_vectors_near_py(v1, v2, tolerance=1e-9):
    assert len(v1) == len(v2), f"Vector length mismatch: {len(v1)} vs {len(v2)}"
    for i in range(len(v1)):
        assert abs(v1[i] - v2[i]) < tolerance, f"Mismatch at index {i}: {v1[i]} vs {v2[i]}"

def test_spectral_embedder_creation_py():
    se = graph_edit_distance.SpectralEmbedder(k=5) # graph_edit_distance.SpectralEmbedder
    assert se is not None
    # k must be int, pybind11 should handle TypeError from C++ string constructor if k is not int
    # std::invalid_argument from C++ (e.g. k=0) maps to RuntimeError in Python
    with pytest.raises(RuntimeError):
         graph_edit_distance.SpectralEmbedder(k=0)
    with pytest.raises(RuntimeError):
         graph_edit_distance.SpectralEmbedder(k=-2)
    # Test default k value if constructor supports it (current C++ default k=5)
    se_default = graph_edit_distance.SpectralEmbedder()
    # We need a way to check k or infer it. For now, just test construction.


def test_spectral_embed_empty_graph_py(spectral_embedder_k3):
    g = Graph()
    emb = spectral_embedder_k3.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 0.0, 0.0])

def test_spectral_embed_single_node_graph_py(spectral_embedder_k3):
    g = Graph()
    g.add_vertex(1, "A")
    # Laplacian of a single node is [[0]]. Eigenvalue is 0.
    emb = spectral_embedder_k3.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 0.0, 0.0]) # Padded

def test_spectral_embed_p2_py(spectral_embedder_k3): # Path graph 1-2
    g = Graph()
    g.add_vertex(1,"A"); g.add_vertex(2,"B")
    g.add_edge(1,2)
    # Eigenvalues of P2 Laplacian: 0, 2
    emb = spectral_embedder_k3.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 2.0, 0.0]) # Padded

def test_spectral_embed_p3_py(spectral_embedder_k3): # Path graph 1-2-3
    g = Graph()
    g.add_vertex(1); g.add_vertex(2); g.add_vertex(3)
    g.add_edge(1,2); g.add_edge(2,3)
    # Eigenvalues of P3 Laplacian: 0, 1, 3
    emb = spectral_embedder_k3.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 1.0, 3.0])

def test_spectral_embed_c3_py(spectral_embedder_k3): # Cycle graph 1-2-3-1
    g = Graph()
    g.add_vertex(1); g.add_vertex(2); g.add_vertex(3)
    g.add_edge(1,2); g.add_edge(2,3); g.add_edge(3,1)
    # Eigenvalues of C3 Laplacian: 0, 3, 3
    emb = spectral_embedder_k3.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 3.0, 3.0])

def test_spectral_embed_padding_py(spectral_embedder_k5): # k=5
    g = Graph() # P2
    g.add_vertex(1); g.add_vertex(2)
    g.add_edge(1,2)
    # Eigenvalues 0, 2. Expected embedding with k=5: [0, 2, 0, 0, 0]
    emb = spectral_embedder_k5.embed_graph(g)
    expect_vectors_near_py(emb, [0.0, 2.0, 0.0, 0.0, 0.0])

def test_spectral_embedding_distance_py():
    emb1 = [0.0, 1.0, 2.0]
    emb2 = [0.0, 1.0, 2.0] # Identical
    emb3 = [1.0, 1.0, 3.0]

    dist_func = graph_edit_distance.SpectralEmbedder.embedding_distance
    assert dist_func(emb1, emb2) == 0.0

    # dist(emb1, emb3) = sqrt((0-1)^2 + (1-1)^2 + (2-3)^2) = sqrt(1 + 0 + 1) = sqrt(2)
    assert abs(dist_func(emb1, emb3) - pow(2.0, 0.5)) < 1e-9

    with pytest.raises(RuntimeError): # std::invalid_argument maps to RuntimeError
        dist_func(emb1, [1.0, 2.0]) # Different sizes

def test_get_node_ordering_public_util_py():
    g = Graph()
    g.add_vertex(3,"C"); g.add_vertex(1,"A"); g.add_vertex(2,"B")
    # Use module-level function bound from static C++ method
    node_order = graph_edit_distance.get_node_ordering_public(g)
    assert node_order == [1,2,3]

def test_get_adjacency_matrix_util_py():
    g = Graph(); g.add_vertex(1); g.add_vertex(2); g.add_edge(1,2)
    # Use static method bound to the Python class
    adj_matrix = graph_edit_distance.SpectralEmbedder.get_adjacency_matrix(g)
    # Adjacency matrix from GraphAlgos is for the directed graph as is
    # Node order will be [1,2]
    #   1 2
    # 1[0,1]
    # 2[0,0]
    assert adj_matrix == [[0.0, 1.0], [0.0, 0.0]]

def test_get_laplacian_matrix_util_py():
    g = Graph(); g.add_vertex(1); g.add_vertex(2); g.add_edge(1,2)
    # Laplacian matrix from GraphAlgos uses an undirected interpretation (symmetric A)
    # Node order [1,2]
    # A_undir = [[0,1],[1,0]], D = [[1,0],[0,1]]
    # L = D - A_undir = [[1,-1],[-1,1]]
    lap_matrix = graph_edit_distance.SpectralEmbedder.get_laplacian_matrix(g)
    assert lap_matrix == [[1.0, -1.0], [-1.0, 1.0]]

# To run these tests:
# 1. Make sure the C++ module is compiled (e.g., `pip install -e .` or `python setup.py build_ext -i`)
# 2. Run `pytest` in the root directory (or `python -m pytest tests/python/`)
#    Pytest should discover and run these tests.
#    Make sure `PYTHONPATH` is set up if needed, or install editable.
#    `pip install -e .` handles this well.
#    Also need `pip install pytest`.
#
# A placeholder requirements-dev.txt or similar could include pytest.
# requirements.txt already has pytest.
# The test file structure is:
# root/
#   src/
#   python/
#     graph_edit_distance/
#       __init__.py
#   tests/
#     cpp/
#       test_graph.cpp
#       test_ged_algorithm.cpp
#       test_graph_algos.cpp
#     python/
#       test_ged_bindings.py
#   setup.py
#   CMakeLists.txt
#
# This looks like a reasonable structure.
# Pytest will find tests/python/test_ged_bindings.py
# The import `import graph_edit_distance` should work if `pip install -e .` was successful.
# The `graph_edit_distance_cpp` module is imported inside `graph_edit_distance/__init__.py`.
# And `Graph`, `Edge`, `EditCosts`, `HeuristicGED`, `SpectralEmbedder` are made available from `graph_edit_distance`.
# This seems correct.
