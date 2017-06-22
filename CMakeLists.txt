# === Set the project name and specify the required CMAKE version.
project (calibration-pipeline CXX C)
cmake_minimum_required (VERSION 2.8)


# === Compiler options.
set (CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -std=c++11")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# === Set cmake 3rd library modules path.
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")


# === General options.
option (ENABLE_PROFILING "Enable/disable profiling" OFF)
option (ENABLE_TESTS "Enable test framework" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
  set (CMAKE_ALL_FLAGS ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set (CMAKE_ALL_FLAGS ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE})
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set (CMAKE_ALL_FLAGS ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG})
endif (CMAKE_BUILD_TYPE STREQUAL "Release")

if (ENABLE_PROFILING)
  set (CMAKE_BUILD_TYPE "Release")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
  link_libraries (profiler)
endif (ENABLE_PROFILING)


# === Project version.
execute_process (
  COMMAND git describe --tags --always
  OUTPUT_VARIABLE VERSION
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (cmake/version.h.in ${PROJECT_BINARY_DIR}/version.h)


# === Include sources.
include (cmake/sources.cmake)
include_directories (${PROJECT_BINARY_DIR})


# === Find and include dependencies.
find_package (Boost 1.40.0 COMPONENTS system)
find_package (GFlags REQUIRED)
find_package (Glog REQUIRED)
find_package (Threads REQUIRED)
find_package (Eigen3 3.1 REQUIRED)
find_package (Pipeline REQUIRED)

include_directories (
  ${Boost_INCLUDE_DIRS} SYSTEM
  ${GFLAGS_INCLUDE_DIRS} SYSTEM
  ${GLOG_INCLUDE_DIRS} SYSTEM
  ${EIGEN3_INCLUDE_DIR} SYSTEM
  ${PIPELINE_INCLUDES} SYSTEM
)


# === Create the binary.
add_executable (aartfaac-calibration ${PIPELINE_SOURCES})

target_link_libraries (aartfaac-calibration
  ${Boost_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)


# === Unit testing.
if (ENABLE_TESTS)
  enable_testing()
  find_package (GTest REQUIRED)
  include_directories (${GTEST_INCLUDE_DIRS} SYSTEM)

  foreach (TEST ${TESTS})
      add_executable (${TEST} ${${TEST}_SOURCES})
      target_link_libraries (${TEST}
        ${GFLAGS_LIBRARIES}
        ${GLOG_LIBRARIES}
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
      )
      add_test (${TEST} ${TEST})
  endforeach (TEST)
endif (ENABLE_TESTS)


# === Install project.
install (DIRECTORY ${PROJECT_SOURCE_DIR}/data/ DESTINATION share/aartfaac)
install (TARGETS aartfaac-calibration RUNTIME DESTINATION bin)


# === Status report.
message (STATUS "")
message (STATUS " C++:")
message (STATUS "   C++ Compiler:         ${CMAKE_CXX_COMPILER}")
message (STATUS "   C++ (Release):        ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message (STATUS "   C++ (Debug):          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
message (STATUS "   C++ (RelWithDebInfo): ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message (STATUS "")
message (STATUS " CMakeflags (${CMAKE_PROJECT_NAME} ${VERSION}):")
message (STATUS "   CMAKE_BUILD_TYPE      ${CMAKE_BUILD_TYPE}")
message (STATUS "   ENABLE_PROFILING      ${ENABLE_PROFILING}")
message (STATUS "   ENABLE_TESTS          ${ENABLE_TESTS}")
message (STATUS "")
