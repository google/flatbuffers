from setuptools import setup

setup(
    name='flatbuffers',
    version='0.1',
    license='BSD',
    author='FlatBuffers Contributors',
    author_email='me@rwinslow.com',
    url='https://github.com/google/flatbuffers/python',
    long_description=('Python runtime library and code generator for use with'
                      'the Flatbuffers serialization format.'),
    packages=['flatbuffers'],
    include_package_data=True,
    requires=[],
    description=('Runtime library and code generator for use with the '
                 'Flatbuffers serialization format.'),
)
