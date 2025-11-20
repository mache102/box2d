#!/usr/bin/env bash
rm -rf build_std
cmake -B build_std -DCMAKE_BUILD_TYPE=Release -DBOX2D_DISABLE_STATIC_DYNAMIC_SPECULATIVE=OFF
cmake --build build_std --target samples
echo "Standard build complete. Run: ./build_std/bin/samples"