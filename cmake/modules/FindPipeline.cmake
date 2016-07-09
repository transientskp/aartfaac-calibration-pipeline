# Find the Pipeline includes and library
#
#  PIPELINE_INCLUDES    - List of header files.
#  PIPELINE_FOUND       - True if found.

if (NOT PIPELINE_FOUND)

    find_path(PIPELINE_INCLUDES pipeline.h
        HINTS
            ${PIPELINE_ROOT}
            $ENV{PIPELINE_ROOT}
        PATH_SUFFIXES
            include/pipeline
    )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Pipeline
        DEFAULT_MSG
        PIPELINE_INCLUDES
    )

    MARK_AS_ADVANCED(PIPELINE_INCLUDES)

endif (NOT PIPELINE_FOUND)
