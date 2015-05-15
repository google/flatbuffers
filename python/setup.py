from setuptools import setup

setup(
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
