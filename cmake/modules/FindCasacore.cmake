# FindCasacore.cmake
# =============================================================================
#
# CMake script to find the Casacore library associated dependencies.
#
# =============================================================================
# Defines the following variables:
#
#   CASACORE_FOUND             = True if Casacore is found.
#   CASACORE_INCLUDES          = Set of include directories needed by Casacore.
#   CASACORE_LIBRARIES         = Set of libraries required for linking.
#
# ============================================================================
# Environment and CMake variables effecting this script.
#
#   CASACORE_ROOT = Top level casacore install directory.
#                         This is the root install location of casacore and
#                         if built from source (on linux) will be usually
#                         be either /usr/local or /usr
#
# ============================================================================
# Known Issues
#   - This script is untested with either Mac OS or Windows.
#
# ============================================================================

if (NOT CASACORE_FOUND)
    set (required_libraries
        casa
        ms
        tables
        images
        lattices
        coordinates
        measures
        scimath
    )

    # For convenience, we want two include directories:
    #
    # 1. ${CASACORE_ROOT}/include
    # 2. ${CASACORE_ROOT}/include/casacore/
    #
    # 1 is convenient for our own code: it lets us #include <casacore/..>
    # 2 is *required* for casacore's internal use.
    find_path(CASACORE_INCLUDES casa/aips.h
        HINTS
            ${CASACORE_ROOT}
        PATH_SUFFIXES
            include/casacore
    )
    if (CASACORE_ROOT)
        list(APPEND CASACORE_INCLUDES ${CASACORE_ROOT}/include)
    endif (CASACORE_ROOT)

    include(FindPackageHandleStandardArgs)
    foreach (lib ${required_libraries})
        find_library (CASACORE_LIB_${lib} casa_${lib}
            HINTS
                ${CASACORE_ROOT}
                $ENV{CASACORE_ROOT}
            PATH_SUFFIXES
                lib
                lib64
        )
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(
            libcasa_${lib}
            DEFAULT_MSG
            CASACORE_LIB_${lib}
        )
        list (APPEND CASACORE_LIBRARIES ${CASACORE_LIB_${lib}})
    endforeach (lib)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Casacore
        DEFAULT_MSG
        CASACORE_INCLUDES
        CASACORE_LIBRARIES
    )

    # Put variables in the advanced section of the CMake cache.
    mark_as_advanced(
        CASACORE_FOUND
        CASACORE_INCLUDES
        CASACORE_LIBRARIES
    )

endif (NOT CASACORE_FOUND)
