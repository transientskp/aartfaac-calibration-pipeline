# Find the Pipeline includes and library
#
#  PIPELINE_INCLUDES    - List of header files.
#  PIPELINE_LIBRARIES   - List of libraries.
#  PIPELINE_FOUND       - True if found.

if (NOT PIPELINE_FOUND)

    find_path(PIPELINE_INCLUDES pipeline.h
        HINTS
            ${PIPELINE_ROOT}
            $ENV{PIPELINE_ROOT}
        PATH_SUFFIXES
            include/pipeline
    )

    find_library(PIPELINE_LIBRARIES pipeline
        HINTS
            ${PIPELINE_ROOT}
            $ENV{PIPELINE_ROOT}
        PATH_SUFFIXES
            lib
            lib64
    )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Pipeline
        DEFAULT_MSG
        PIPELINE_LIBRARIES
        PIPELINE_INCLUDES
    )

    MARK_AS_ADVANCED(PIPELINE_LIBRARIES PIPELINE_INCLUDES)

endif (NOT PIPELINE_FOUND)
