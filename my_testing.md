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



## SetPistonVelocity

let's add a function b2Body_SetPistonVelocity(b2BodyId bodyId, b2Vec2 velocity)
the function applies the velocity to the body for one tick, and then resets 'piston' velocity to zero.

The function deviates from SetLinearVelocity, such that:

- velocity must be in one of the 4 cardinal directions (+-x, +-y), otherwise ignored  
- example of ignored velocity: b2Vec2(1, 1), b2Vec2(-1, -1), b2Vec2(0, 0) 
- example of valid velocity: b2Vec2(1, 0), b2Vec2(-3, 0), b2Vec2(0, 5), b2Vec2(0, -6)

for each dynamic body, M, that would be affected by the piston's movement from (x,y) to (x+v.x, y+v.y):
- M's position can only be updated along the axis of the piston's movement. i.e. if piston moves in +-x, M's x is updated and y is as is, and vice versa.
- if the piston moves in +x dir: find largest x such that M 'collides'
- if the piston moves in -x dir: find smallest x such that M 'collides'
- if the piston moves in +y dir: find largest y such that M 'collides'
- if the piston moves in -y dir: find smallest y such that M 'collides' 


'collides' definition: M collides with at least one of the shapes that are part of the piston body. 
most piston bodies only have 1 shape; some have up to 2 shapes. all shapes are either circles or convex polygons with few vertices.

example: a square piston body moves from (3, 0) to (6, 0). if a dynamic body M was at (4.5, 0), then its resulting position would need to be to the right of the piston body, i.e. somewhere at x > 6.

after finding the new position, M is moved to that position. M's velocity should be left unchanged.



### Test Sample
The "Kinematic Velocity Transfer" benchmark demonstrates this feature:
- Two kinematic boxes (at x=0 and x=2)
- Two dynamic boxes above them

- Press **K** to apply upward velocity to both kinematic boxes for one tick. when done:
  - Left dynamic box (red): applies piston velocity
  - Right dynamic box (green): applies linear velocity
- Observe: the green box jumps (acquires velocity), the red box is pushed up (position correction) but stops immediately when kinematic stops

## Running Unit Tests (with speculative disabled)

```bash
cmake -B build -DBUILD_SHARED_LIBS=OFF -DBOX2D_UNIT_TESTS=ON -DBOX2D_VALIDATE=ON -DBOX2D_DISABLE_STATIC_DYNAMIC_SPECULATIVE=ON

cmake --build build --config Debug


   
./build/bin/test              # Linux/macOS  
./build/bin/Debug/test.exe    # Windows
```
**Important:** The Falling Hinge unit test fails due to changes in collision handling. Several `Speculative` integration tests in `samples/` also fail (i.e. the dynamic body phases through the static body).

These failures are expected, in return for reduced ghost collision severity.