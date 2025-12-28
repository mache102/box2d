# Box2D Feature Addition: b2Body_SetPistonVelocity

## Goal

Add a new API function:

b2Body_SetPistonVelocity(b2BodyId bodyId, b2Vec2 velocity)

This function applies a one-tick, axis-aligned “piston” movement to a body and pushes overlapping dynamic bodies along the same axis. After the tick completes, the piston velocity is reset to zero.

The implementation reuses existing Box2D collision detection and CCD/TOI logic, with minimal new code.

---

## Velocity constraints

The input velocity is valid only when it is strictly axis-aligned.

Valid:
(v, 0) where v ≠ 0  
(0, v) where v ≠ 0  

Ignored:
(0, 0)  
(x, y) where both components are non-zero  

When the velocity is ignored, the function performs no action.

---

## Scope of affected bodies

For the piston body P, consider dynamic bodies that may be affected by the piston’s movement from:

P.position → P.position + velocity

Potential interaction during this movement is identified using the same CCD / TOI logic already used by Box2D for fast-moving shapes.

---

## Collision definition

A dynamic body M is considered colliding if at least one of its fixtures intersects at least one fixture of the piston body at any point during the piston’s movement.

Notes:
- Piston bodies typically have 1 fixture, sometimes up to 2
- Fixtures are circles or convex polygons with few vertices
- Existing shape-vs-shape overlap or distance tests are reused

---

## Push behavior (strictly 1D)

Once a dynamic body M is identified as affected, its position is updated only along the piston’s axis of motion. The perpendicular axis remains unchanged.

Axis rules:

for a piston moving in an axis (+x, -x, +y, or -y): 

- move M in the same direction, until it no longer overlaps the piston AND is greater than the piston's position along that axis

This is a single-axis positional solve. A piston moving in the x axis affects M.x, while a piston moving in the y axis affects M.y.

---

## Example

A square piston body moves from (3, 0) to (6, 0).  
A dynamic body M is initially at (4.5, 0).

After the piston movement:
- M.x is moved to a position strictly greater than 6
- M.y is unchanged
- M’s linear velocity is unchanged

---

## Implementation approach

Existing CCD / TOI code paths are used to determine which dynamic bodies are affected by the piston’s movement.

A dedicated 1D positional solver is implemented to compute the final position of each affected body along the movement axis. The solver projects along the axis and finds the earliest non-overlapping configuration.

The implementation is localized, deterministic, and integrates cleanly with existing Box2D systems.

---

## Expected outcome

- Piston movement behaves deterministically
- Dynamic bodies are pushed cleanly along one axis
- Velocities of pushed bodies remain unchanged
- Behavior resembles Minecraft-style pistons
- Code changes are minimal and easy to audit
