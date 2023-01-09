* [ ] Change contact normal to be local **!!!ASAP!!!** (only box-box is correct).
* [ ] Change contact ref point to be local **!!!ASAP!!!** (only box-box is correct).
* [x] Implement asking mem. manager for new pool allocator.
* [ ] Stack allocator for single frame allocations (contact in physics engine, etc.).
* [x] Implement 'should collide/intersect' for broad/narrow phase
  (at least as a z coordinate comparison, **filtering / grouping** is **MUCH** better)
  **upd:** used filtering + grouping.
* [x] Remove explicit position / rotation from rigid body,
  and use Transform2D (component?) instead.
* [x] Fixed rotation option for rigid bodies.
* [ ] Abandon box for general polygon?
* [x] Generate multiple contact points.
* [x] Abandon narrow phase for contact manager? (yes?) **upd:** decided not to.
* [ ] Word wrap in text rendering.
* [x] Move rigidbody/collider to **Physics** namespace to distinguish it from components.
* [x] Extract uvs for individual sprites in spritesheet.
* [x] Colliders are not stored in rb.
* [x] Both colliders and rb has reference/ pointer to transform2d.
* [x] When rb is destroyed it's colliders are orphaned, and to deleted.
* [ ] Fix narrow phase not generating contacts for colliders only.
* [ ] Fix weird behaviour when collider is offset from [0,0] and rotation is not constrained.
* [ ] Compile-time map.
* [ ] Fix DRY in SceneGraph and ScenePanels (FindTopLevel... and MarkHierarchy...).
* [x] Create prefabs: ordinary scene descriptions, but when:
    * Deserializing:
        * Create entity that represents prefab, add `Prefab` component (or something like that).
        * Deserialize as ordinary, adding `BelongsToPrefab` component to all entities.
    * Serializing:
        * Serialize entity with `Prefab` component (something like `Prefab: Name: <name>, Path: <path>`)
        * Do no serialize entities with `BelongsToPrefab` component.
* [ ] Fix 'Ghost collisions'.
* [ ] Change view: instead of keeping `reference pool` keep reference to dense vectors directly.
* [ ] Return multiple colliders to rigid body.