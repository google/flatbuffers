# -*- coding: utf-8 -*-
import sys
import glob
import shutil

from setuptools import setup

try:
    from Cython.Build import cythonize
    from Cython.Distutils import build_ext
except ImportError:
    extra_kwds = dict()
else:
    from setuptools import Extension

    if sys.platform == "win32":
        extra_compile_args = ['/EHsc']
    elif sys.platform == "darwin":
        extra_compile_args = ['-std=c++11']
    else:
        extra_compile_args = ['-std=c++0x']
    extra_kwds = dict(
        ext_modules=cythonize([
            Extension('flatbuffers.fastcodec',
                      sources=['flatbuffers/fastcodec.pyx'],
                      include_dirs=['.'],
                      extra_compile_args=extra_compile_args,
                      language='c++')
        ]),
        cmdclass={'build_ext': build_ext},
    )


# copy `*.h` as package data, source distribution need these files
for cxx_source_file in glob.glob('../include/flatbuffers/*.h'):
    shutil.copy(cxx_source_file, 'flatbuffers')


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
setup_kwds.update(extra_kwds)
setup(**setup_kwds)
