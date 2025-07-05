"""
simple_usage.py

This script demonstrates basic usage of the graph_edit_distance library, including:
1. Creating graph objects.
2. Calculating heuristic Graph Edit Distance (GED).
3. Generating and comparing graph embeddings using Spectral Embedding.
"""

try:
    import graph_edit_distance as ged
except ImportError:
    print("Error: The 'graph_edit_distance' module could not be imported.")
    print("Please ensure the library is correctly built and installed.")
    print("Try running 'pip install -e .' from the project root directory.")
    exit(1)

def main():
    print("--- Graph Edit Distance Library: Simple Usage Example ---")

    # 1. Create graphs
    print("\nStep 1: Creating Graphs...")
    g1 = ged.Graph()
    g1.add_vertex(1, "A")
    g1.add_vertex(2, "B")
    g1.add_edge(1, 2, "edge_1_2")
    print(f"Graph g1: {g1}")
    g1.print_graph() # More detailed C++ printout

    g2 = ged.Graph()
    g2.add_vertex(10, "A")  # Node 10 in g2 corresponds to node 1 in g1
    g2.add_vertex(20, "X")  # Node 20 in g2 is different from node 2 in g1 (label mismatch)
    g2.add_edge(10, 20, "edge_10_20")
    print(f"\nGraph g2: {g2}")
    g2.print_graph()

    # 2. Calculate Heuristic GED
    print("\nStep 2: Calculating Heuristic Graph Edit Distance...")
    # Define edit costs (default: all operations cost 1.0)
    # For this example, let's use default costs explicitly for clarity
    costs = ged.EditCosts(
        vertex_insertion_cost=1.0,
        vertex_deletion_cost=1.0,
        vertex_substitution_cost=1.0, # Cost if labels differ
        edge_insertion_cost=1.0,
        edge_deletion_cost=1.0
    )
    print(f"Using EditCosts: {costs!r}") # Print repr of costs

    heuristic_calculator = ged.HeuristicGED(costs)
    distance = heuristic_calculator.compute_distance(g1, g2)
    print(f"Heuristic GED between g1 and g2: {distance}")
    # Expected behavior with these costs and graphs:
    # - Map vertex 1 (A) from g1 to vertex 10 (A) from g2: cost = 0 (labels match)
    # - Map vertex 2 (B) from g1 to vertex 20 (X) from g2: cost = 1 (labels "B" and "X" differ, vertex_substitution_cost)
    # Total vertex operation cost = 1.0.
    # - Edge (1,2) in g1 maps to edge (10,20) in g2.
    #   The current heuristic checks for existence of the mapped edge. If edge labels were considered for substitution,
    #   and "edge_1_2" != "edge_10_20", an edge substitution cost would apply if defined.
    #   Assuming simple edge presence/absence: edge (10,20) exists, so 0 cost for this transformed edge.
    # Total GED = 1.0.

    # 3. Use Spectral Embedding for Graph Similarity
    print("\nStep 3: Using Spectral Embedding for Graph Similarity...")
    k_eigenvalues = 3  # Number of eigenvalues for the embedding
    embedder = ged.SpectralEmbedder(k=k_eigenvalues)
    print(f"SpectralEmbedder initialized with k={k_eigenvalues}")

    emb1 = embedder.embed_graph(g1)
    emb2 = embedder.embed_graph(g2)

    print(f"Embedding for g1 (k={k_eigenvalues}): {emb1}")
    print(f"Embedding for g2 (k={k_eigenvalues}): {emb2}")

    # Calculate distance between embeddings
    # Note: embedding_distance is a static method of SpectralEmbedder
    embedding_dist = ged.SpectralEmbedder.embedding_distance(emb1, emb2)
    print(f"Euclidean distance between g1 and g2 embeddings: {embedding_dist:.4f}")

    # Example with a slightly different graph for comparison
    g3 = ged.Graph()
    g3.add_vertex(100, "NodeAlpha")
    g3.add_vertex(200, "NodeBeta")
    g3.add_vertex(300, "NodeGamma")
    g3.add_edge(100, 200)
    g3.add_edge(200, 300) # g3 is a path graph P3
    print(f"\nGraph g3: {g3}")
    g3.print_graph()

    emb3 = embedder.embed_graph(g3)
    print(f"Embedding for g3 (k={k_eigenvalues}): {emb3}")

    embedding_dist_g1_g3 = ged.SpectralEmbedder.embedding_distance(emb1, emb3)
    print(f"Distance between g1 and g3 embeddings: {embedding_dist_g1_g3:.4f}")

    embedding_dist_g2_g3 = ged.SpectralEmbedder.embedding_distance(emb2, emb3)
    print(f"Distance between g2 and g3 embeddings: {embedding_dist_g2_g3:.4f}")

    print("\n--- Example script finished ---")

if __name__ == "__main__":
    main()
