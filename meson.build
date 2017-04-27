project('FlatBuffers', 'cpp',
    meson_version : '>=0.40.0',
    default_options : ['cpp_std=c++11'])

FlatBuffers_Library_SRCS = [
    'include/flatbuffers/code_generators.h',
    'include/flatbuffers/flatbuffers.h',
    'include/flatbuffers/hash.h',
    'include/flatbuffers/idl.h',
    'include/flatbuffers/util.h',
    'include/flatbuffers/reflection.h',
    'include/flatbuffers/reflection_generated.h',
    'include/flatbuffers/flexbuffers.h',
    'src/code_generators.cpp',
    'src/idl_parser.cpp',
    'src/idl_gen_text.cpp',
    'src/reflection.cpp',
    'src/util.cpp']

FlatBuffers_Compiler_SRCS = [
    FlatBuffers_Library_SRCS,
    'src/idl_gen_cpp.cpp',
    'src/idl_gen_general.cpp',
    'src/idl_gen_go.cpp',
    'src/idl_gen_js.cpp',
    'src/idl_gen_php.cpp',
    'src/idl_gen_python.cpp',
    'src/idl_gen_fbs.cpp',
    'src/idl_gen_grpc.cpp',
    'src/flatc.cpp',
    'src/flatc_main.cpp',
    'grpc/src/compiler/schema_interface.h',
    'grpc/src/compiler/cpp_generator.h',
    'grpc/src/compiler/cpp_generator.cc',
    'grpc/src/compiler/go_generator.h',
    'grpc/src/compiler/go_generator.cc']

FlatHash_SRCS = [
    'include/flatbuffers/hash.h',
    'src/flathash.cpp']

CPP_Flags = [
    '-Wall',
    '-pedantic',
    '-Werror',
    '-Wextra']

compiler = meson.get_compiler('cpp')

# Certain platforms such as ARM do not use signed chars by default
# which causes issues with certain bounds checks.
if compiler.has_argument('-fsigned-char')
   CPP_Flags += '-fsigned-char'
endif

include_dirs = include_directories(['include', 'grpc'])

flatbuffers_lib = static_library('flatbuffers',
    FlatBuffers_Library_SRCS,
    include_directories : include_dirs,
    cpp_args : CPP_Flags)

flatc = executable('flatc',
    FlatBuffers_Compiler_SRCS,
    include_directories : include_dirs,
    cpp_args : CPP_Flags)

flathash = executable('flathash',
    FlatHash_SRCS,
    include_directories : include_dirs,
    cpp_args : CPP_Flags)
