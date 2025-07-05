# Graph Edit Distance Library

A Python library with C++ backend for computing graph edit distance (GED).

This library aims to provide:
- Efficient computation of GED using C++.
- A heuristic benchmark algorithm.
- A graph embedding based approach for GED approximation.

## Features

- **Core Graph Data Structures (C++ & Python):**
    - Represents directed graphs with labeled vertices and edges.
    - Comprehensive API for graph manipulation (add/remove vertices/edges, set/get labels).
- **Python Bindings:**
    - C++ core exposed to Python using `pybind11`.
- **Graph Edit Distance (GED) Calculation:**
    - **Heuristic Algorithm:** A greedy algorithm to compute an approximate GED score.
    - **Customizable Costs:** `EditCosts` class allows defining costs for vertex/edge insertion, deletion, and substitution.
- **Graph Embedding for Similarity:**
    - **Spectral Embedding:** `SpectralEmbedder` class generates graph embeddings using the smallest `k` eigenvalues of the graph Laplacian.
    - Distance between embeddings can be used as a measure of graph dissimilarity.
- **Testing:**
    - Comprehensive C++ unit tests using Google Test.
    - Python unit tests using `pytest` to verify bindings and functionality.

## Installation

The library is built using CMake and setuptools. Ensure you have a C++ compiler (supporting C++17), CMake, and Python development headers.

1.  **Clone the repository (if applicable):**
    ```bash
    git clone <repository_url>
    cd <repository_name>
    ```

2.  **Install Python dependencies (includes `pybind11` and `pytest`):**
    ```bash
    pip install -r requirements.txt
    ```

3.  **Build and install the C++ extension (editable mode recommended for development):**
    ```bash
    pip install -e .
    ```
    This command invokes `setup.py`, which in turn uses CMake to compile the C++ code and create the Python extension module.

## Quick Usage Example (Python)

```python
import graph_edit_distance as ged

# 1. Create graphs
g1 = ged.Graph()
g1.add_vertex(1, "A")
g1.add_vertex(2, "B")
g1.add_edge(1, 2, "edge_1_2")

g2 = ged.Graph()
g2.add_vertex(10, "A") # Node 10 in g2 corresponds to node 1 in g1
g2.add_vertex(20, "X") # Node 20 in g2 is different from node 2 in g1
g2.add_edge(10, 20, "edge_10_20")

print("Graph 1:", g1)
print("Graph 2:", g2)

# 2. Calculate Heuristic GED
# Define edit costs (or use default: all operations cost 1.0)
costs = ged.EditCosts(vertex_substitution_cost=1.0,
                      vertex_deletion_cost=1.0, vertex_insertion_cost=1.0,
                      edge_deletion_cost=1.0, edge_insertion_cost=1.0)
heuristic_calculator = ged.HeuristicGED(costs)
distance = heuristic_calculator.compute_distance(g1, g2)
print(f"Heuristic GED between g1 and g2: {distance}")
# Expected:
# Map 1(A) -> 10(A) (cost 0)
# Substitute 2(B) with 20(X) (cost 1, vertex_substitution_cost)
# Edge (1,2) maps to (10,20). Edge labels differ or not considered by default heuristic for substitution.
# If edge (10,20) exists in G2, cost is 0 for edge part.
# Total heuristic GED is typically 1.0 in this case with default costs.

# 3. Use Spectral Embedding for Graph Similarity
# Embed graphs (using k=3 smallest Laplacian eigenvalues)
embedder = ged.SpectralEmbedder(k=3)

emb1 = embedder.embed_graph(g1)
emb2 = embedder.embed_graph(g2)

print(f"Embedding for g1 (k=3): {emb1}")
print(f"Embedding for g2 (k=3): {emb2}")

# Calculate distance between embeddings
embedding_dist = ged.SpectralEmbedder.embedding_distance(emb1, emb2)
print(f"Distance between graph embeddings: {embedding_dist}")

# Example with a slightly different graph for comparison
g3 = ged.Graph()
g3.add_vertex(1, "A")
g3.add_vertex(2, "B")
g3.add_vertex(3, "C")
g3.add_edge(1,2)
g3.add_edge(2,3)
emb3 = embedder.embed_graph(g3)
print(f"Embedding for g3 (k=3): {emb3}")
embedding_dist_1_3 = ged.SpectralEmbedder.embedding_distance(emb1, emb3)
print(f"Distance between g1 and g3 embeddings: {embedding_dist_1_3}")
```

## Running Tests

Ensure `pytest` is installed (`pip install pytest`).

1.  **Build the extension:**
    ```bash
    pip install -e .
    # or python setup.py build_ext --inplace
    ```
2.  **Run C++ tests (optional, requires build files):**
    Navigate to your CMake build directory (e.g., `build/`) and run:
    ```bash
    ctest
    # Or run the cpp_tests executable directly: ./cpp_tests (or Release/cpp_tests.exe on Windows)
    ```
3.  **Run Python tests:**
    From the project root directory:
    ```bash
    pytest
    ```

## Future Development (Planned)

- More sophisticated GED algorithms (e.g., A* based optimal or better approximations).
- Advanced graph embedding techniques (e.g., GNN-based).
- Support for larger graph datasets and performance optimizations.
- More detailed documentation and examples.
- Packaging for PyPI.
