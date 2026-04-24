# v0.0.0

## 2026-04-23

### 2. Added BenchmarkChainSegmentGhost benchmark

**Commit hash:** `5adf823`

#### User-facing summary

#### Technical changes
- Added `BenchmarkChainSegmentGhost` to `samples/sample_benchmark.cpp` cloning `BenchmarkGroundGhost` parameters (SEG_LEN=GROUND_W=1.0f; BALL_RADIUS=0.25f; BALL_START={0,0}; FORCE=50.0f)
- Loop uses CCW winding: N_MAIN+1 top vertices go WEST (right→left) so `b2RightPerp` gives upward normal; drop vertex at bottom-left; diagonal return closes back to top-right
- Each segment created via `b2CreateChainSegmentShape`; ghost vertices set with `b2ChainSegment_SetGhostVertices` using modular index arithmetic for the closed loop
- Tracks tick; current pos; max height reached; liftoff x — no speculative-collision compile flag line

### 1. Added b2CreateChainSegmentShape and ghost vertex API

**Commit hash:** `5f322982b1b72d49201d1738349f7c84d6da6321`

#### User-facing summary

#### Technical changes
- Added `b2CreateChainSegmentShape` to `src/shape.c` and `include/box2d/box2d.h` — creates a `b2ChainSegment` shape attached to a body with `chainId = B2_NULL_INDEX` and default ghost vertices
- Added `b2ChainSegment_SetGhostVertices` to `src/shape.c` and `include/box2d/box2d.h` — sets ghost1/ghost2 on an existing chain segment shape by shapeId
- Added `BenchmarkChainSegmentRoll` to `samples/sample_benchmark.cpp` — closed loop of N_MAIN+2 chain segments with correct ghost vertices; ball rolls along the top surface
