* [ ] Change contact normal to be local **!!!ASAP!!!** (only box-box is correct).
* [ ] Change contact ref point to be local **!!!ASAP!!!** (only box-box is correct).
* [ ] Implement asking mem. manager for new pool allocator.
* [ ] Stack allocator for single frame allocations (contact in physics engine, etc.).
* [x] Implement 'should collide/intersect' for broad/narrow phase 
    (at least as a z coordinate comparison, **filtering / grouping** is **MUCH** better) 
    **upd:** used filtering + grouping.
* [ ] Remove explicit position / rotation from rigid body,
    and use Transform2D (component?) instead.
* [x] Fixed rotation option for rigid bodies.
* [ ] Abandon box for general polygon?
* [x] Generate multiple contact points.
* [x] Abandon narrow phase for contact manager? (yes?) **upd:** decided not to
* [ ] Word wrap in text rendering
* [ ] Move rigidbody/collider to **Physics** namespace to distinguish it from components.
* [ ] Extract uvs for individual sprites in spritesheet.