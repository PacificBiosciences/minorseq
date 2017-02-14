#!/bin/bash
set -euo pipefail

# Main script
echo "# LOAD MODULES"
source /mnt/software/Modules/current/init/bash
module load git gcc/4.9.2 python/2.7.9 cmake cram/0.7 swig ccache virtualenv zlib/1.2.5 ninja boost

echo "# PRE-BUILD HOOK"
echo "## Check formatting"
./tools/check-formatting --all

echo "# BUILD"
echo "## Create build directory "
if [ ! -d build ] ; then mkdir build ; fi

echo "## Build source"
( cd build &&\
  rm -rf * &&\
  CMAKE_BUILD_TYPE=ReleaseWithAssert cmake -DJULIET_INHOUSE_PERFORMANCE=T -GNinja .. )
( cd build && ninja )

echo "## tests"
( cd build && ninja check )