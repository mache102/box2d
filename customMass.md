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

### Collision pipeline trace (from contact confirmation to solved state)

This is the end-to-end flow for solid contact processing in the current codebase, with focus on where inverse mass is read and how it modifies final state.

1. **Narrow-phase contact confirmation and per-contact mass setup (`src/physics_world.c`, `b2CollideTask`)**
- Broad overlap check (`b2AABB_Overlaps`) decides whether contact remains potentially active.
- For overlapping pairs, `b2UpdateContact(...)` updates manifold and touching state.
- Before solver prep, contact-side cached solver values are written:
  - `contactSim->bodySimIndexA/B`
  - `contactSim->invMassA/B`
  - `contactSim->invIA/IB`
- Custom collision mass hook is here:
  - For dynamic-dynamic only: `contactSim->invMassA /= scaleA; contactSim->invMassB /= scaleB;`
  - This means collision responses use effective inverse masses (linear mass sharing), while inertia stays unchanged.

2. **Constraint graph / awake sets carry contact sims to solver (`src/constraint_graph.c`)**
- Contact sims are stored into graph colors (or overflow color).
- Static bodies are represented with zero inverse mass/inertia in constraints (`invMass=0`, `invI=0`, `bodySimIndex=B2_NULL_INDEX`).

3. **Solver stage order (`src/solver.c`)**
- `PrepareContacts` -> `IntegrateVelocities` -> `WarmStart` -> `Solve` -> `IntegratePositions` -> `Relax` -> `Restitution` -> `StoreImpulses`.
- Overflow contacts run in scalar path; colored contacts run SIMD path. Both use the same physical variables and equations.

4. **Prepare contact constraints (`src/contact_solver.c`, `b2PrepareContactsTask` / overflow equivalent)**
- Reads `contactSim->invMassA/B` and `invIA/IB` into constraint storage (`constraint->invMassA/B`, `constraint->invIA/IB`).
- Computes per-point effective normal/tangent masses:
  - `kNormal = mA + mB + iA*rnA^2 + iB*rnB^2`
  - `normalMass = 1 / kNormal`
  - same pattern for tangent.
- Stores `relativeVelocity` for restitution decisions later.

5. **Warm start applies cached impulses to velocities (`b2WarmStartContactsTask`)**
- Re-applies prior frame impulses directly into body velocities:
  - linear: `vA -= invMassA * P`, `vB += invMassB * P`
  - angular: `wA -= invIA * cross(rA,P)`, `wB += invIB * cross(rB,P)`
- This is the first direct place where `invMassA/B` modifies current-step velocity state.

6. **Main contact solve updates velocities from new impulses (`b2SolveContactsTask`)**
- For each point and sub-step:
  - Computes separation and relative normal velocity `vn`.
  - Solves incremental normal impulse (with bias/softness and clamping).
  - Applies impulse to velocities:
    - `vA -= invMassA * Pn`, `vB += invMassB * Pn`
    - `wA -= invIA * cross(rA,Pn)`, `wB += invIB * cross(rB,Pn)`
  - Solves friction impulse similarly and applies with same `invMassA/B`.
- Therefore, changing `contactSim->invMassA/B` linearly scales how much each body’s **linear velocity** changes per impulse.

7. **Position deltas are integrated from solved velocities (`src/solver.c`, `b2IntegratePositionsTask`)**
- Uses solved `state->linearVelocity/angularVelocity` to advance:
  - `state->deltaPosition += h * state->linearVelocity`
  - `state->deltaRotation = IntegrateRotation(...)`
- So collision-driven velocity differences become collision-driven position correction/motion over sub-steps.

8. **Relax pass**
- Runs another contact solve pass without bias (`useBias=false`), still applying impulses through `invMassA/B` to velocities.

9. **Restitution pass (`b2ApplyRestitutionTask` / overflow equivalent)**
- If restitution is active and thresholds pass, computes bounce impulse from stored `relativeVelocity`.
- Applies bounce impulse again through:
  - `vA -= invMassA * P`, `vB += invMassB * P`
  - angular terms via `invIA/IB`.

10. **Finalize bodies writes back final transforms (`src/solver.c`, `b2FinalizeBodiesTask`)**
- Final state application:
  - `sim->center += state->deltaPosition`
  - `sim->transform.q = normalize(deltaRotation * sim->transform.q)`
  - `sim->transform.p = sim->center - rotate(q, localCenter)`
- Resets force/torque accumulators for next step.
- AABBs are updated from new transforms.

### How inverse mass reaches final modified variables (summary)

- `contactSim->invMassA/B` (set in narrow phase) -> copied into constraints in prepare stage.
- In warm start + solve + restitution, impulses are converted to velocity deltas using `deltaV = invMass * impulse`.
- These updated `linearVelocity`/`angularVelocity` values are integrated into `deltaPosition`/`deltaRotation`.
- Finalization commits deltas into body transform/center.

So the custom collision mass scale affects collisions by scaling inverse mass at contact setup, which directly scales impulse-to-velocity conversion, and then indirectly scales resulting positional movement through velocity integration.
