# ------------------- Debianization ---------------------
if (UNIX)

    # Set build environment
    SET(CPACK_GENERATOR "TGZ;DEB")
    SET(CPACK_SOURCE_TGZ "ON")

    # Common package information
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "FlatBuffers is an efficient cross platform serialization library for C++, with support for Java, C# and Go. It was created at Google specifically for game development and other performance-critical applications.")
    SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/google/flatbuffers")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vitaly Isaev <vitalyisaev2@gmail.com>")

    # Derive package version from git
    EXECUTE_PROCESS(
        COMMAND date +%Y%m%d
        OUTPUT_VARIABLE DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
      COMMAND git describe
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_DESCRIBE_DIRTY
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" VERSION_COMMIT "${GIT_DESCRIBE_DIRTY}")
    SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
    SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
    SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
    SET(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_COMMIT}")
    SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")

    # Derive architecture
    IF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
      FIND_PROGRAM(DPKG_CMD dpkg)
      IF(NOT DPKG_CMD)
        MESSAGE(STATUS "Can not find dpkg in your path, default to i386.")
        SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
      ENDIF(NOT DPKG_CMD)
      EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
        OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ENDIF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

    # Package name
    SET(CPACK_DEBIAN_PACKAGE_NAME "flatbuffers")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
    SET(CPACK_PACKAGE_FILE_NAME 
        "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

endif(UNIX)

INCLUDE(CPack)
