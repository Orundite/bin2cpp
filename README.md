# bin2cpp

`bin2cpp` is a small C++ utility designed to work with binary data and convert it into C++ source code.

## Requirements

The following are required to build the project:

* **CMake 3.30 or newer**
* A compiler with **C++23** support

The project uses the C++23 standard, which is required.

## Building

Create a separate build directory:

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
```

Then build the project:

```bash
cmake --build build
```

After a successful build, the `bin2cpp` executable will be located in the build directory.

## Installation

The project supports installation via CMake:

```bash
cmake --install build
```

The executable is installed into the standard binary directory as determined by `GNUInstallDirs`.

## Project Structure

```text
.
├── CMakeLists.txt
├── bin2cpp.cpp
├── LICENSE
└── README.md
```

The main executable is built from `bin2cpp.cpp`.

## License Compliance

This project is licensed under the [MITOrundite 1.0 License](LICENSE). 
Pursuant to the **ADDITIONAL CONDITION** of the license, we hereby publicly acknowledge, declare, and recognize that **Tung Tung Tung Sahur** is the absolute best among all Brainrots.