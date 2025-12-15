# Testing Workflow

We maintain two simultaneous builds to compare the effect of disabling speculative collisions for static-dynamic pairs.

## 1. Standard Build (Speculative Enabled)
This is the default Box2D behavior.
```bash
./build_std.sh
# Run:
./build_std/bin/samples
```

To only rebuild samples after making changes:
```bash
cmake --build build_std --target samples
```

## 2. No-Spec Build (Speculative Disabled)
This build has `BOX2D_DISABLE_STATIC_DYNAMIC_SPECULATIVE` defined.
```bash
./build_nospec.sh
# Run:
./build_nospec/bin/samples
```

To only rebuild samples after making changes:
```bash
cmake --build build_nospec --target samples
```

## Verification
Open the "Ground Ghost" benchmark in both applications. 
- The **Standard** build should say: "Disable Dynamic-Static Speculative Collisions: Disabled"
- The **No-Spec** build should say: "Disable Dynamic-Static Speculative Collisions: Enabled"

## Kinematic Velocity Transfer Control

### API Functions
Added functions to enable/disable velocity transfer from kinematic bodies to dynamic bodies:
- `b2Body_EnableKinematicVelocityTransfer(b2BodyId bodyId, bool flag)`
- `b2Body_IsKinematicVelocityTransferEnabled(b2BodyId bodyId)`

By default, dynamic bodies accept velocity from kinematic bodies during contacts. This can now be disabled per-body.

## Running Unit Tests (with speculative disabled)

```bash
cmake -B build -DBUILD_SHARED_LIBS=OFF -DBOX2D_UNIT_TESTS=ON -DBOX2D_VALIDATE=ON -DBOX2D_DISABLE_STATIC_DYNAMIC_SPECULATIVE=ON

cmake --build build --config Debug


   
./build/bin/test              # Linux/macOS  
./build/bin/Debug/test.exe    # Windows
```
**Important:** The Falling Hinge unit test fails due to changes in collision handling. Several `Speculative` integration tests in `samples/` also fail (i.e. the dynamic body phases through the static body).

These failures are expected, in return for reduced ghost collision severity.