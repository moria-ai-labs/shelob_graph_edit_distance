#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For automatic conversion of STL containers
#include <pybind11/operators.h> // For operator overloading

#include "Graph.h" // Our Graph class header
#include "EditCosts.h"
#include "GEDAlgorithm.h"   // For HeuristicGED
#include "GraphAlgos.h"     // For SpectralEmbedder

namespace py = pybind11;
namespace ged = ged; // Alias for convenience

PYBIND11_MODULE(graph_edit_distance_cpp, m) {
    m.doc() = "Python bindings for the C++ Graph Edit Distance library core"; // Optional module docstring

    // Bind the Label type alias if it's simple like std::string
    // If Label were a complex struct, it would need its own binding.
    // For std::string, pybind11 handles it automatically.

    // Bind the Edge struct
    py::class_<Edge>(m, "Edge")
        .def(py::init<>()) // Default constructor
        .def(py::init<VertexID, const Label&>(), py::arg("to"), py::arg("label") = "")
        .def_readwrite("to", &Edge::to)
        .def_readwrite("label", &Edge::label)
        .def(py::self < py::self) // For comparison, if used in Python sets/sorted lists
        .def("__repr__",
             [](const Edge &e) {
                 return "<Edge to=" + std::to_string(e.to) + " label='" + e.label + "'>";
             });

    // Bind the Graph class
    py::class_<Graph>(m, "Graph")
        .def(py::init<>()) // Constructor
        .def("add_vertex", &Graph::add_vertex, py::arg("id"), py::arg("label") = "",
             "Adds a vertex to the graph.")
        .def("remove_vertex", &Graph::remove_vertex, py::arg("id"),
             "Removes a vertex and its incident edges from the graph.")
        .def("set_vertex_label", &Graph::set_vertex_label, py::arg("id"), py::arg("label"),
             "Sets the label of an existing vertex.")
        .def("get_vertex_label", &Graph::get_vertex_label, py::arg("id"),
             "Gets the label of a vertex. Throws an exception if vertex not found.")
        .def("has_vertex", &Graph::has_vertex, py::arg("id"),
             "Checks if a vertex exists in the graph.")
        .def("get_vertices", &Graph::get_vertices,
             "Returns a set of all vertex IDs in the graph.")
        .def("num_vertices", &Graph::num_vertices,
             "Returns the number of vertices in the graph.")

        .def("add_edge", &Graph::add_edge, py::arg("u"), py::arg("v"), py::arg("label") = "",
             "Adds a directed edge from vertex u to vertex v.")
        .def("remove_edge", &Graph::remove_edge, py::arg("u"), py::arg("v"),
             "Removes a directed edge from vertex u to vertex v.")
        .def("set_edge_label", &Graph::set_edge_label, py::arg("u"), py::arg("v"), py::arg("label"),
             "Sets the label of an existing directed edge (u,v).")
        .def("get_edge_label", &Graph::get_edge_label, py::arg("u"), py::arg("v"),
             "Gets the label of a directed edge (u,v). Throws an exception if edge not found.")
        .def("has_edge", &Graph::has_edge, py::arg("u"), py::arg("v"),
             "Checks if a directed edge (u,v) exists.")
        .def("get_edges_from", &Graph::get_edges_from, py::arg("u"),
             "Returns a set of outgoing edges from vertex u.")
        .def("get_all_edges", &Graph::get_all_edges,
             "Returns a list of pairs (u,v) for all directed edges in the graph.")
        .def("num_edges", &Graph::num_edges,
             "Returns the number of directed edges in the graph.")

        .def("print_graph", &Graph::print_graph,
             "Prints a representation of the graph to standard output (for debugging).")

        // Pythonic helpers
        .def("__len__", &Graph::num_vertices, "Returns the number of vertices (len(graph)).")
        .def("__contains__", &Graph::has_vertex, "Checks if a vertex ID is in the graph (vertex_id in graph).")
        .def("__str__", [](const Graph &g) {
            // A more Pythonic string representation could be implemented
            // For now, let's just indicate it's a graph object
            // Or, we could capture the output of print_graph if desired, but that's more complex.
            return "<Graph with " + std::to_string(g.num_vertices()) + " vertices and " +
                   std::to_string(g.num_edges()) + " edges>";
        });

    // Bind EditCosts struct
    py::class_<EditCosts>(m, "EditCosts")
        .def(py::init<double, double, double, double, double>(),
             py::arg("vertex_insertion_cost") = 1.0,
             py::arg("vertex_deletion_cost") = 1.0,
             py::arg("vertex_substitution_cost") = 1.0,
             py::arg("edge_insertion_cost") = 1.0,
             py::arg("edge_deletion_cost") = 1.0)
        .def_readwrite("vertex_insertion_cost", &EditCosts::vertex_insertion_cost)
        .def_readwrite("vertex_deletion_cost", &EditCosts::vertex_deletion_cost)
        .def_readwrite("vertex_substitution_cost", &EditCosts::vertex_substitution_cost)
        .def_readwrite("edge_insertion_cost", &EditCosts::edge_insertion_cost)
        .def_readwrite("edge_deletion_cost", &EditCosts::edge_deletion_cost)
        .def("__repr__", [](const EditCosts &c) {
            return "<EditCosts: vi=" + std::to_string(c.vertex_insertion_cost) +
                   ", vd=" + std::to_string(c.vertex_deletion_cost) +
                   ", vs=" + std::to_string(c.vertex_substitution_cost) +
                   ", ei=" + std::to_string(c.edge_insertion_cost) +
                   ", ed=" + std::to_string(c.edge_deletion_cost) + ">";
        });

    // Bind HeuristicGED class (from ged namespace)
    py::class_<ged::HeuristicGED>(m, "HeuristicGED")
        .def(py::init<const EditCosts&>(), py::arg("costs"))
        .def("compute_distance", &ged::HeuristicGED::compute_distance, py::arg("g1"), py::arg("g2"),
             "Computes the Graph Edit Distance using a heuristic algorithm.");

    // For binding static matrix functions, we need a way to get node ordering.
    // Let's assume SpectralEmbedder has a public static method `get_node_ordering_public`.
    // If not, these static bindings might need to be removed or adapted.
    // (This was `get_node_ordering` and was private. For binding, it needs to be public or called internally)
    // I will add a placeholder `get_node_ordering_public` to GraphAlgos.h and .cpp for this binding.

    py::class_<ged::SpectralEmbedder>(m, "SpectralEmbedder")
        .def(py::init<int>(), py::arg("k") = 5,
             "Initializes the spectral embedder with k eigenvalues to compute.")
        .def("embed_graph", &ged::SpectralEmbedder::embed_graph, py::arg("graph"),
             "Generates an embedding for the graph using Laplacian eigenvalues.")
        .def_static("embedding_distance", &ged::SpectralEmbedder::embedding_distance,
                    py::arg("emb1"), py::arg("emb2"),
             "Computes Euclidean distance between two embedding vectors.")
        // The static matrix functions are bound here assuming get_node_ordering_public exists
        .def_static("get_adjacency_matrix", [](const Graph& g) {
            std::vector<VertexID> node_order = ged::SpectralEmbedder::get_node_ordering_public(g);
            if (g.num_vertices() > 0 && node_order.empty() && g.num_vertices() != node_order.size() ) { // Check consistency
                 throw std::runtime_error("Could not determine consistent node ordering for matrix construction.");
            }
            return ged::SpectralEmbedder::get_adjacency_matrix(g, node_order);
        }, py::arg("graph"), "Computes the adjacency matrix (as list of lists).")
        .def_static("get_laplacian_matrix", [](const Graph& g) {
            std::vector<VertexID> node_order = ged::SpectralEmbedder::get_node_ordering_public(g);
            if (g.num_vertices() > 0 && node_order.empty() && g.num_vertices() != node_order.size()) { // Check consistency
                 throw std::runtime_error("Could not determine consistent node ordering for matrix construction.");
            }
            return ged::SpectralEmbedder::get_laplacian_matrix(g, node_order);
        }, py::arg("graph"), "Computes the Laplacian matrix (as list of lists).");

    // Expose the helper itself if useful
    m.def("get_node_ordering_public", &ged::SpectralEmbedder::get_node_ordering_public,
          py::arg("graph"), "Utility to get a sorted list of vertex IDs from a graph for matrix construction.");

    // Version information (optional)
    #ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
    #else
        m.attr("__version__") = "dev";
    #endif
}
