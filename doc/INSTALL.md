*Note: all are welcome to build our software from source, but official
 support is only provided for official builds provided by PacBio
 (the SMRT Analysis suite)*

 ***

Building from scratch requires system-wide installed boost (>=1.58.0), 
cmake (3.2), and a c++11 compiler (>=gcc-4.9, clang):

  ```sh
  git clone https://github.com/PacificBiosciences/minorseq && cd minorseq
  git submodule update --init --remote
  mkdir build && cd build
  cmake -GNinja -DCMAKE_INSTALL_PREFIX=~/bin .. && ninja
  ```