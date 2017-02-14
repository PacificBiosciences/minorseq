
include(CheckCXXCompilerFlag)

# shared CXX flags for all source code & tests
set(MS_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")

# gperftools support
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND APPLE)
    set(MS_LINKER_FLAGS "${MS_LINKER_FLAGS} -Wl,-no_pie")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug" AND APPLE)

# static linking
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(MS_LINKER_FLAGS "${MS_LINKER_FLAGS} -static-libgcc -static-libstdc++")
ENDIF()

# NOTE: quash clang warnings w/ Boost
check_cxx_compiler_flag("-Wno-unused-local-typedefs" HAS_NO_UNUSED_LOCAL_TYPEDEFS)
if(HAS_NO_UNUSED_LOCAL_TYPEDEFS)
    set(MS_FLAGS "${MS_FLAGS} -Wno-unused-local-typedefs")
endif()

# Cannot use this until pbbam complies
# if (CMAKE_COMPILER_IS_GNUCXX)
#     set(MS_FLAGS "${MS_FLAGS} -Werror=suggest-override")
# endif()

# Coverage settings
if (MS_inc_coverage)
    set(MS_COV_FLAGS "${MS_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# Extra testing that will lead to longer compilation times!
if (SANITIZE)
    # AddressSanitizer is a fast memory error detector
    set(MS_SANITY_FLAGS "${MS_SANITY_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")

    # Clang Thread Safety Analysis is a C++ language extension which warns about
    # potential race conditions in code.
    set(MS_SANITY_FLAGS "${MS_SANITY_FLAGS} -Wthread-safety")

    # ThreadSanitizer is a tool that detects data races
    set(MS_SANITY_FLAGS "${MS_SANITY_FLAGS} -fsanitize=thread")

    # MemorySanitizer is a detector of uninitialized reads.
    set(MS_SANITY_FLAGS "${MS_SANITY_FLAGS} -fsanitize=memory")

    # UndefinedBehaviorSanitizer is a fast undefined behavior detector.
    set(MS_SANITY_FLAGS "${MS_SANITY_FLAGS} -fsanitize=undefined")
endif()

# shared CXX flags for src & tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MS_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MS_COV_FLAGS} ${MS_SANITY_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${MS_LINKER_FLAGS}")


# Set additionally needed linker flags
if(APPLE)
    set(PRE_LINK -force_load)
elseif(UNIX)
    SET(PRE_LINK -Wl,-whole-archive)
    SET(POST_LINK -Wl,-no-whole-archive)
endif()

if (JULIET_INHOUSE_PERFORMANCE)
    add_definitions(-DJULIET_INHOUSE_PERFORMANCE)
endif()