#!/usr/bin/env bash
rm -rf build_nospec
cmake -B build_nospec -DCMAKE_BUILD_TYPE=Release -DBOX2D_DISABLE_STATIC_DYNAMIC_SPECULATIVE=ON
cmake --build build_nospec --target samples
echo "No-Spec build complete. Run: ./build_nospec/bin/samples"