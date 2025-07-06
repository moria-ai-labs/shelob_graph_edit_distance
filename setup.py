import os
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import subprocess

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        super().__init__(name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        if not isinstance(ext, CMakeExtension):
            super().build_extension(ext)
            return

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
        ]
        build_args = []

        if self.compiler.compiler_type == "msvc":
            # Single config generators are handled CMakeList.txt
            cmake_args += [
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
            ]
            # Specify generator and platform if not specified
            if cmake_generator == "" and PLAT_TO_CMAKE[self.plat_name]:
                 cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # build_args += ["--config", cfg] # Not needed for MSVC

        else: # Ninja or Unix Makefiles
            # Multi-config generators pass an additional flag
            if cmake_generator == "Ninja Multi-Config":
                cmake_args += [
                    f"-DCMAKE_CONFIGURATION_TYPES={cfg}",
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}" # Needed for multi-config
                ]
                build_args += ["--config", cfg]


        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS Intel on Apple Silicon
            archs = os.environ.get("ARCHFLAGS", "")
            if "x86_64" in archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES=x86_64"]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in setup.py build
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        build_temp = self.build_temp + os.path.sep + ext.name
        if not os.path.exists(build_temp):
            os.makedirs(build_temp)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=build_temp
        )

# Placeholder for the C++ extension module
# We will define the actual sources and include_dirs later
# For now, we assume the CMakeLists.txt in ./src will handle the pybind11 details.
ext_modules = [
    CMakeExtension("graph_edit_distance_cpp", sourcedir="."),
]

setup(
    name="graph_edit_distance",
    version="0.0.1",
    author="AI Agent Jules", # Placeholder
    author_email="jules@example.com", # Placeholder
    description="A Python library with C++ backend for graph edit distance computation and graph embedding.",
    long_description=open("README.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/user/graph-edit-distance-library", # Placeholder URL
    keywords=[
        "graph edit distance", "GED", "graph similarity", "graph matching",
        "spectral embedding", "graph algorithms", "pybind11"
    ],
    ext_modules=ext_modules,
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.7",
    setup_requires=[ # pybind11 is needed to build, not just at runtime by final user
        "pybind11>=2.6"
    ],
    install_requires=[ # For users of the library, pybind11 might not be strictly needed if they just run compiled code
        "pybind11>=2.6" # Keep for now, or decide if only build-time
    ],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License", # Choose an appropriate license
        "Operating System :: OS Independent",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Bio-Informatics", # Example topic
        "Topic :: Scientific/Engineering :: Information Analysis",
    ],
    # Define where the Python package is (so `import graph_edit_distance` works)
    # This assumes your Python wrapper code will be in `python/graph_edit_distance/`
    packages=["graph_edit_distance"],
    package_dir={"": "python"},
)
