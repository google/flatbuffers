# -*- coding: utf-8 -*-
import os
import sys
import platform
import glob
import shutil

from distutils import errors
from setuptools import Extension, setup

try:
    from Cython.Distutils import build_ext
except ImportError:
    has_cython = False
else:
    has_cython = True

    class BuildFailed(Exception):
        """Raised when building Cython extension failed."""

        def __init__(self):
            # work around py 2/3 different syntax
            self.cause = sys.exc_info()[1]

    class ve_build_ext(build_ext):
        """This class allows Cython extension building to fail."""

        def run(self):
            try:
                build_ext.run(self)
            except errors.DistutilsPlatformError:
                raise BuildFailed()

        def build_extension(self, ext):
            try:
                build_ext.build_extension(self, ext)
            except ext_errors:
                raise BuildFailed()
            except ValueError:
                # this can happen on Windows 64 bit, see Python issue 7511
                # works with both py 2/3
                if "'path'" in str(sys.exc_info()[1]):
                    raise BuildFailed()
                raise


cpython = platform.python_implementation() == 'CPython'

if sys.platform == "win32":
    extra_compile_args = ['/EHsc']  # catch C++ exceptions
elif sys.platform == "darwin":
    extra_compile_args = ['-std=c++11']
else:
    extra_compile_args = ['-std=c++0x']

ext_modules = [
    Extension('flatbuffers.fastcodec',
              sources=['flatbuffers/fastcodec.pyx'],
              include_dirs=['.'],
              extra_compile_args=extra_compile_args,
              language='c++'),
]

ext_errors = (errors.CCompilerError,
              errors.DistutilsExecError,
              errors.DistutilsPlatformError)
if sys.platform == 'win32':
    # 2.6's distutils.msvc9compiler can raise an IOError when failing to
    # find the compiler
    ext_errors += (IOError,)


# copy `*.h` as package data, source distribution need these files
for cxx_source_file in glob.glob('../include/flatbuffers/*.h'):
    shutil.copy(cxx_source_file, 'flatbuffers')


def status_msgs(*msgs):
    print('*' * 75)
    for msg in msgs:
        print(msg)
    print('*' * 75)


def run_setup(with_cext):
    setup_kwds = dict(
        name='flatbuffers',
        version='2015.05.14.0',
        license='Apache 2.0',
        author='FlatBuffers Contributors',
        author_email='me@rwinslow.com',
        url='https://github.com/google/flatbuffers',
        long_description=('Python runtime library for use with the Flatbuffers'
                          'serialization format.'),
        packages=['flatbuffers'],
        include_package_data=True,
        requires=[],
        description='The FlatBuffers serialization format for Python',
    )

    if with_cext:
        setup_kwds.update(
            ext_modules=ext_modules,
            cmdclass={'build_ext': ve_build_ext},
        )
    setup(**setup_kwds)


if not cpython:
    run_setup(False)
    status_msgs(
        "Cython extensions are not supported on this Python platform.",
        "Plain-Python build succeeded."
    )
elif os.environ.get('DISABLE_FLATBUFFERS_CEXT'):
    run_setup(False)
    status_msgs(
        "DISABLE_FLATBUFFERS_CEXT is set; " +
        "not attempting to build Cython extensions.",
        "Plain-Python build succeeded."
    )
elif not has_cython:
    run_setup(False)
    status_msgs(
        "Cython does not appear to be installed; " +
        "not attempting to build Cython extensions.",
        "Plain-Python build succeeded."
    )
else:
    try:
        run_setup(True)
    except BuildFailed as exc:
        status_msgs(
            exc.cause,
            "WARNING: The Cython extension could not be compiled, " +
            "speedups are not enabled.",
            "Failure information, if any, is above.",
            "Retrying the build without the Cython extension now."
        )

        run_setup(False)
        status_msgs(
            "WARNING: The Cython extension could not be compiled, " +
            "speedups are not enabled.",
            "Plain-Python build succeeded."
        )
