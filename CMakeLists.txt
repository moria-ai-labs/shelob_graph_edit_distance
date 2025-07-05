cmake_minimum_required(VERSION 3.12)
project(GraphEditDistance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Pybind11 setup ---
# Find pybind11
# Option 1: Find a pybind11 installation using find_package
# find_package(pybind11 CONFIG REQUIRED)

# Option 2: Use pybind11 cloned as a submodule or included in the source tree
# add_subdirectory(extern/pybind11) # If pybind11 is in extern/pybind11

# Option 3: Fetch pybind11 using FetchContent (CMake 3.11+)
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.10.4 # Or any other stable tag/commit
)
FetchContent_MakeAvailable(pybind11)
# --- End Pybind11 setup ---


# --- Python interpreter ---
# This is usually handled by setup.py passing -DPYTHON_EXECUTABLE
# but we can add a fallback or ensure it's found
find_package(PythonInterp REQUIRED)
# find_package(PythonLibs REQUIRED) # May be needed on some systems


# --- Project source files ---
set(PROJECT_SOURCES
    src/Graph.cpp
    src/GEDAlgorithm.cpp
    src/GraphAlgos.cpp
    src/bindings.cpp
)

# --- pybind11 module ---
# The name here MUST match the first argument to CMakeExtension in setup.py: "graph_edit_distance_cpp"
pybind11_add_module(graph_edit_distance_cpp SHARED ${PROJECT_SOURCES})

target_include_directories(graph_edit_distance_cpp PRIVATE
    ${pybind11_INCLUDE_DIRS} # From pybind11
    # ${PYTHON_INCLUDE_DIRS} # Usually included via pybind11_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src" # To find Graph.h
    ${EIGEN3_INCLUDE_DIR} # For Eigen library
)

# If pybind11 is built as a static library or shared library (less common for header-only usage)
# you might need to link against it. pybind11_LIBRARIES handles this.
# For modern pybind11, it's often an INTERFACE library, so linking is about properties.
target_link_libraries(graph_edit_distance_cpp PRIVATE
    ${pybind11_LIBRARIES} # Includes pybind11::module or pybind11::embed
    # ${PYTHON_LIBRARIES} # Usually included via pybind11_LIBRARIES
)

# Set properties for Python module
set_target_properties(graph_edit_distance_cpp PROPERTIES
    OUTPUT_NAME "graph_edit_distance_cpp" # Ensures correct output name
    # PREFIX "" # On some systems, helps remove "lib" prefix
    # SUFFIX ".pyd" # On Windows, or ".so" on Linux/macOS - pybind11 handles this
)

# Ensure the output directory is correctly set for setup.py to find the module
# CMAKE_LIBRARY_OUTPUT_DIRECTORY is passed from setup.py
# For multi-config generators, CMAKE_LIBRARY_OUTPUT_DIRECTORY_<CONFIG> is used.
# pybind11_add_module usually handles this, but explicit setting can be a fallback.
# set_target_properties(graph_edit_distance_cpp PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
# )
# if(CMAKE_CONFIGURATION_TYPES) # For multi-config generators like VS, Xcode, Ninja Multi-Config
#     foreach(config ${CMAKE_CONFIGURATION_TYPES})
#         string(TOUPPER "${config}" config_upper)
#         set_target_properties(graph_edit_distance_cpp PROPERTIES
#             LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper}}"
#         )
#     endforeach()
# endif()


# --- Installation ---
# Installation rules for the Python module are typically handled by setup.py,
# which copies the compiled .so/.pyd file from the build directory
# to the appropriate site-packages or editable install location.

# --- Testing (Optional: C++ tests with CTest) ---
enable_testing()

# Google Test
find_package(GTest QUIET) # Ensure GTest is found before Eigen if it also uses FetchContent
if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1 # Or a more recent release
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  message(STATUS "GTest not found system-wide. Fetched using FetchContent.")
else()
  message(STATUS "Found GTest: ${GTest_DIR}")
endif()

# --- Eigen3 library for matrix operations ---
find_package(Eigen3 QUIET)
if(NOT Eigen3_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0 # Or another stable tag
    )
    FetchContent_MakeAvailable(eigen)
    # Eigen is header-only, but this makes Eigen3::Eigen available as a target
    # If using Eigen < 3.3, you might need to manually add Eigen_INCLUDE_DIRS
    set(EIGEN3_INCLUDE_DIR ${eigen_SOURCE_DIR}) # For older CMake/Eigen versions
    message(STATUS "Eigen3 not found system-wide. Fetched using FetchContent.")
else()
    message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIRS}")
    set(EIGEN3_INCLUDE_DIR ${EIGEN3_INCLUDE_DIRS}) # Ensure variable is set for consistency
endif()
# --- End Eigen3 library ---


# Define C++ tests if the test source directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/cpp/test_graph.cpp") # Check for a specific file or just dir
    add_executable(cpp_tests
        tests/cpp/test_graph.cpp
        tests/cpp/test_ged_algorithm.cpp
        tests/cpp/test_graph_algos.cpp # New test file
        src/Graph.cpp          # Link Graph implementation
        src/GEDAlgorithm.cpp   # Link GEDAlgorithm implementation
        src/GraphAlgos.cpp     # Link GraphAlgos implementation
    )
    target_link_libraries(cpp_tests PRIVATE GTest::gtest GTest::gtest_main Eigen3::Eigen)
     target_include_directories(cpp_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        ${EIGEN3_INCLUDE_DIR} # For Eigen library
     )

    include(GoogleTest)
    gtest_discover_tests(cpp_tests)
    message(STATUS "C++ tests configured.")
else()
    message(WARNING "C++ test file (e.g., tests/cpp/test_graph.cpp) not found. Skipping C++ test configuration.")
endif()


message(STATUS "Python Executable: ${PYTHON_EXECUTABLE}")
message(STATUS "pybind11 include dirs: ${pybind11_INCLUDE_DIRS}")
message(STATUS "pybind11 found: ${pybind11_FOUND}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target graph_edit_distance_cpp created.")
# message(STATUS "Output directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
# get_target_property(module_location graph_edit_distance_cpp LIBRARY_OUTPUT_DIRECTORY)
# message(STATUS "Module location for graph_edit_distance_cpp: ${module_location}")
# get_target_property(module_location_release graph_edit_distance_cpp LIBRARY_OUTPUT_DIRECTORY_RELEASE)
# message(STATUS "Module location for graph_edit_distance_cpp (Release): ${module_location_release}")
