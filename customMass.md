## Custom mass

functions to set and get a custom mass for a body.

this custom mass only affects dynamic-dynamic collisions.

we'll need to add these box2d functions to the API:

1. a function that sets the custom mass for a body. if not set, the custom mass is 1.0.
2. a function that gets the custom mass for a body. if not set, it returns 1.0.

so what happens with both bodies colliding are b2BodyType.b2_dynamicBody? their resolution vectors will be affected by the custom masses comparison. if custom masses are the same, then resolution remains as is. if one mass is greater than the other, than the smaller mass body will have its resolution vector multiplied by the ratio of the smaller mass to the greater mass. this will make the smaller mass body move more than the greater mass body, simulating a more realistic collision response based on their custom masses.
this custom mass should be given a different name from the existing mass and density values.

### Implementation plan (appended)

#### Naming options (to avoid confusion with physical mass/density)

Recommended primary name:
- `collisionMassScale` (API: `b2Body_SetCollisionMassScale`, `b2Body_GetCollisionMassScale`)

Alternative names:
- `contactMassScale`
- `dynamicContactMassScale`
- `collisionWeight`

Why `collisionMassScale` is best:
- It is explicit that this is a collision-response tuning scalar, not geometric mass/density.
- It is dimensionless and naturally defaults to `1.0f`.
- It mirrors existing Box2D naming style (`SetX` / `GetX`) on `b2Body`.

#### Code changes needed

1. **Body storage**
- Add a new `float collisionMassScale;` to internal `b2Body` (`src/body.h`).
- Initialize it to `1.0f` in `b2CreateBody` (`src/body.c`).

2. **Public API**
- Add to `include/box2d/box2d.h`:
  - `void b2Body_SetCollisionMassScale(b2BodyId, float);`
  - `float b2Body_GetCollisionMassScale(b2BodyId);`
- Enforce valid values in setter (`> 0`, finite) with the project’s assert style.
- Getter returns `1.0f` by default because bodies are initialized with `1.0f`.

3. **Collision solver integration**
- Hook in `src/physics_world.c` where `contactSim->invMassA/B` are prepared.
- Only for **dynamic-dynamic** body pairs:
  - Read `collisionMassScaleA/B`.
  - Convert to effective inverse mass via `invMass /= collisionMassScale`.
  - This makes lower scale => more movement, higher scale => less movement.
- Leave static/kinematic interactions unchanged.
- Leave inertia terms unchanged (feature is scoped to linear collision mass sharing).

4. **Tests**
- Add a world/unit test in `test/test_world.c`:
  - verify default getter = `1.0f`
  - verify set/get round-trip
  - create overlapping dynamic-dynamic bodies with asymmetric scales and verify the lower scale body gets larger separation movement.

5. **Validation**
- Build and run unit tests to ensure no regressions.
