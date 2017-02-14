# Config generation

find_git_sha1(MINORSEQ_GIT_SHA1)

file (STRINGS "${MS_RootDir}/CHANGELOG.md" MINORSEQ_CHANGELOG)

configure_file(
    ${MS_IncludeDir}/pacbio/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/pacbio/Version.h
)
