set(FlatBuffers_known_comps static shared)
set(FlatBuffers_comp_static NO)
set(FlatBuffers_comp_shared NO)
foreach(FlatBuffers_comp IN LISTS ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
    if(FlatBuffers_comp IN_LIST FlatBuffers_known_comps)
        set(FlatBuffers_comp_${FlatBuffers_comp} YES)
    else()
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "FlatBuffers does not recognize component `${FlatBuffers_comp}`.")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif()
endforeach()

if(FlatBuffers_comp_static AND FlatBuffers_comp_shared)
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
        "FlatBuffers `static` and `shared` components are mutually exclusive.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    return()
endif()

set(FlatBuffers_static_targets "${CMAKE_CURRENT_LIST_DIR}/FlatBuffersStaticTargets.cmake")
set(FlatBuffers_shared_targets "${CMAKE_CURRENT_LIST_DIR}/FlatBuffersSharedTargets.cmake")

macro(FlatBuffers_load_targets type)
    if(NOT EXISTS "${FlatBuffers_${type}_targets}")
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "FlatBuffers `${type}` libraries were requested but not found.")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif()
    include("${FlatBuffers_${type}_targets}")
endmacro()

if(FlatBuffers_comp_static)
    FlatBuffers_load_targets(static)
elseif(FlatBuffers_comp_shared)
    FlatBuffers_load_targets(shared)
elseif(DEFINED FlatBuffers_SHARED_LIBS AND FlatBuffers_SHARED_LIBS)
    FlatBuffers_load_targets(shared)
elseif(DEFINED FlatBuffers_SHARED_LIBS AND NOT FlatBuffers_SHARED_LIBS)
    FlatBuffers_load_targets(static)
elseif(BUILD_SHARED_LIBS)
    if(EXISTS "${FlatBuffers_shared_targets}")
        FlatBuffers_load_targets(shared)
    else()
        FlatBuffers_load_targets(static)
    endif()
else()
    if(EXISTS "${FlatBuffers_static_targets}")
        FlatBuffers_load_targets(static)
    else()
        FlatBuffers_load_targets(shared)
    endif()
endif()

# flatbuffers never materializes a separate "_shared"-suffixed physical
# library; flatbuffers::flatbuffers is the only real target. This alias
# exists purely so that consumers hardcoded to the legacy
# flatbuffers::flatbuffers_shared name keep working when the target we just
# loaded happens to be shared. ALIAS of an IMPORTED target needs CMake 3.11+;
# older consumers just don't get the compatibility alias.
if(NOT CMAKE_VERSION VERSION_LESS 3.11 AND NOT TARGET flatbuffers::flatbuffers_shared)
    get_target_property(FlatBuffers_imported_type flatbuffers::flatbuffers TYPE)
    if(FlatBuffers_imported_type STREQUAL "SHARED_LIBRARY")
        add_library(flatbuffers::flatbuffers_shared ALIAS flatbuffers::flatbuffers)
    endif()
    unset(FlatBuffers_imported_type)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/FlatcTargets.cmake" OPTIONAL)
include("${CMAKE_CURRENT_LIST_DIR}/BuildFlatBuffers.cmake" OPTIONAL)
