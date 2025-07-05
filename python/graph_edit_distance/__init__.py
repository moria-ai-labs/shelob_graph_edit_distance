"""
Graph Edit Distance (GED) Python Library
========================================

This library provides tools for graph manipulation, heuristic calculation of
Graph Edit Distance, and spectral graph embedding for similarity analysis.
The core functionalities are implemented in C++ for performance and exposed
to Python using pybind11.

Available Classes:
  Graph: Represents a directed graph with labeled vertices and edges.
  Edge: Represents an edge in the Graph.
  EditCosts: Configuration for costs of edit operations (vertex/edge CUD).
  HeuristicGED: Computes an approximate GED using a greedy heuristic.
  SpectralEmbedder: Generates graph embeddings using Laplacian eigenvalues.

Example:
  >>> import graph_edit_distance as ged
  >>> g = ged.Graph()
  >>> g.add_vertex(1, "A")
  >>> print(g)
  <Graph with 1 vertices and 0 edges>
"""

# Import the C++ extension module
# The name 'graph_edit_distance_cpp' should match what's defined in setup.py and CMakeLists.txt
try:
    from .graph_edit_distance_cpp import *
    # You might want to explicitly import specific functions/classes
    # e.g., from .graph_edit_distance_cpp import calculate_ged, Graph
except ImportError as e:
    # This can happen if the C++ extension hasn't been built yet
    # Or if there's an issue with the dynamic library loading (e.g. on Windows PATH missing)
    print(f"Graph Edit Distance: Could not import C++ extension: {e}")
    print("Please ensure the library is compiled and installed correctly.")
    # Optionally, re-raise the error or define Python fallbacks if any

__version__ = "0.0.1"

# You can also define Python-level classes or functions here if needed,
# which might use the C++ backend.

def hello_python_world():
    """A simple function to test package import."""
    return "Hello from graph_edit_distance Python package!"

# To make functions/classes directly available under `import graph_edit_distance`
# e.g., graph_edit_distance.hello_python_world()
# and later graph_edit_distance.calculate_ged()

# If you use `from .graph_edit_distance_cpp import *`,
# then C++ functions will be available directly.
# Otherwise, you might do:
# from . import graph_edit_distance_cpp
# and users would call graph_edit_distance.graph_edit_distance_cpp.some_function()
# or you would alias them:
# some_cpp_function = graph_edit_distance_cpp.some_cpp_function
# __all__ = ["hello_python_world", "some_cpp_function"] # if you define __all__ explicitly.
