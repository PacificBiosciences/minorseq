# functions
function(create_exe libName)
    add_executable(${libName} ${MS_SourceDir}/tools/main/${libName}.cpp)

    set_target_properties(${libName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    target_link_libraries(${libName}
        ${ARGV1}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
        minorseq
    )

    install(TARGETS ${libName} RUNTIME DESTINATION bin)
endfunction()

# get all header files for IDE support
file(GLOB_RECURSE MS_HEADER "${MS_IncludeDir}/*.h")
file(GLOB_RECURSE MS_HIDDEN_HEADER "*.h")
# get sources for src/
file(GLOB MS_CPP        "*.cpp")
# get sources for src/tools
file(GLOB MS_TOOLS_CPP  "tools/*.cpp")

# add main library including everything

add_library(minorseq STATIC
    ${MS_HEADER}
    ${MS_HIDDEN_HEADER}
    ${ssw_INCLUDE_DIRS}/ssw_cpp.cpp
    ${ssw_INCLUDE_DIRS}/ssw.c
    ${MS_CPP}
    ${MS_TOOLS_CPP}
)

target_link_libraries(minorseq
    ${ZLIB_LIBRARIES}
    ${HTSLIB_LIBRARIES}
    ${PacBioBAM_LIBRARIES}
    ${pbcopper_LIBRARIES}
)

target_include_directories(minorseq PUBLIC
    ${MS_IncludeDir}
    ${CMAKE_BINARY_DIR}/generated
    ${Boost_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${HTSLIB_INCLUDE_DIRS}
    ${PacBioBAM_INCLUDE_DIRS}
    ${pbcopper_INCLUDE_DIRS}
    ${ssw_INCLUDE_DIRS}
)

create_exe(juliet)
create_exe(fuse)
create_exe(cleric)