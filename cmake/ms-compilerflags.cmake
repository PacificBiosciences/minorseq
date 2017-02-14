
include(CheckCXXCompilerFlag)

# shared CXX flags for all source code & tests
set(MS_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")

# static linking
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(MS_LINK_FLAGS "${MS_LINK_FLAGS} -static-libgcc -static-libstdc++")
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
    set(MS_FLAGS "${MS_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# Extra testing that will lead to longer compilation times!
if (SANITIZE)
    # AddressSanitizer is a fast memory error detector
    set(MS_FLAGS "${MS_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")

    # Clang Thread Safety Analysis is a C++ language extension which warns about
    # potential race conditions in code.
    set(MS_FLAGS "${MS_FLAGS} -Wthread-safety")

    # ThreadSanitizer is a tool that detects data races
    set(MS_FLAGS "${MS_FLAGS} -fsanitize=thread")

    # MemorySanitizer is a detector of uninitialized reads.
    set(MS_FLAGS "${MS_FLAGS} -fsanitize=memory")

    # UndefinedBehaviorSanitizer is a fast undefined behavior detector.
    set(MS_FLAGS "${MS_FLAGS} -fsanitize=undefined")
endif()

if (JULIET_INHOUSE_PERFORMANCE)
    set(MS_FLAGS "${MS_FLAGS} -DJULIET_INHOUSE_PERFORMANCE")
endif()

# shared CXX flags for src & tests
SET_PROPERTY(GLOBAL PROPERTY MINORSEQ_COMPILE_FLAGS_GLOBAL ${MS_FLAGS})
SET_PROPERTY(GLOBAL PROPERTY MINORSEQ_LINK_FLAGS_GLOBAL ${MS_LINK_FLAGS})