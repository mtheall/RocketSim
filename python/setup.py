#!/usr/bin/env python3

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import os
import numpy

sources = []
headers = []

for root, dirs, files in os.walk("."):
    for file in files:
        if file.endswith(".cpp"):
            sources.append(os.path.join(root, file))
        elif file.endswith(".h"):
            headers.append(os.path.join(root, file))

for root, dirs, files in os.walk("../libsrc"):
    for file in files:
        if file.endswith(".cpp"):
            sources.append(os.path.join(root, file))
        elif file.endswith(".h"):
            headers.append(os.path.join(root, file))

for root, dirs, files in os.walk("../src"):
    for file in files:
        if file.endswith(".cpp"):
            sources.append(os.path.join(root, file))
        elif file.endswith(".h"):
            headers.append(os.path.join(root, file))

class build_ext_ex(build_ext):
    extra_compile_args = {
        "unix": ["-std=c++20", "-flto", "-g0"],
        "msvc": ["/std:c++20", "/GL"]
    }

    extra_link_args = {
        "unix": ["-flto"],
        "msvc": ["/LTCG"]
    }

    def build_extension(self, ext):
        ext.extra_compile_args = self.extra_compile_args.get(self.compiler.compiler_type, [])
        ext.extra_link_args    = self.extra_link_args.get(self.compiler.compiler_type, [])

        build_ext.build_extension(self, ext)

RocketSim = Extension(
    name = "RocketSim",
    sources = sources,
    depends = headers,
    include_dirs = ["../libsrc", "../src", numpy.get_include()],
    language = "c++",
    py_limited_api = True,
    define_macros = [
        ("MAJOR_VERSION", "1"),
        ("MINOR_VERSION", "0"),
        ("PY_SSIZE_T_CLEAN", "1"),
        ("Py_LIMITED_API", "0x03040000"),
        ("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION"),
        ("PY_ARRAY_UNIQUE_SYMBOL", "RocketSim_ARRAY_API"),
        ("NO_IMPORT_ARRAY", "1"),
        ("RS_DONT_LOG", "1")
    ]
)

setup(
    name = "RocketSim",
    version = "1.0",
    description = "This is Rocket League!",
    cmdclass = {"build_ext": build_ext_ex},
    ext_modules = [RocketSim]
)
