# AGENTS.md

## General Guidelines for AI Agents

- **Project Goal:** Develop a high-performance Python library with a C++ backend for calculating Graph Edit Distance (GED).
- **Core Technologies:** Python, C++, pybind11.
- **Development Phases:**
    1.  Project setup and basic structure.
    2.  C++ core graph representation and edit operations.
    3.  Python bindings using pybind11.
    4.  Heuristic GED algorithm implementation (benchmark).
    5.  Graph embedding based GED methodology.
    6.  Comprehensive testing (C++ and Python).
    7.  Documentation and packaging.

## Coding Conventions

- **C++:**
    - Follow modern C++ standards (C++17 or later if possible).
    - Use clear and descriptive naming conventions.
    - Comment complex logic.
    - Prefer smart pointers over raw pointers for memory management where appropriate.
- **Python:**
    - Follow PEP 8 guidelines.
    - Use type hints.
    - Write clear docstrings for modules, classes, and functions.
- **pybind11:**
    - Keep binding code clean and well-organized.
    - Ensure Pythonic interfaces for C++ functionalities.

## Testing

- **C++:** Use a standard C++ testing framework (e.g., Google Test, Catch2). Aim for good test coverage of core logic.
- **Python:** Use `pytest`. Test the Python API, bindings, and integration with the C++ backend.
- Run tests frequently during development.

## Build System

- The primary build system for C++ and bindings will be managed through `setup.py` using `pybind11`'s integration with setuptools.
- Ensure `pip install -e .` works smoothly.

## Version Control

- Commit messages should be clear and follow conventional formats (e.g., imperative mood for the subject line).
- Create logical commits that group related changes.

## Dependencies

- Minimize external dependencies where possible.
- Clearly list all dependencies in `requirements.txt` (for Python) and document C++ dependencies if any are added beyond standard libraries and pybind11.

## Communication

- If a step in the plan is unclear or blocked, please ask for clarification.
- Provide updates on progress and any significant changes to the plan.
