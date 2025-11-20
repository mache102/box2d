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