# TODO LIST
* [ ] Body owns colliders, when creating collider w/o body, static body is automatically created (handled by scene).
* [ ] `CompositeCollier` is either `ChainCollider` or `PolygonCollider`.
* [x] Bodies and colliders stored as lists, these lists allocate from dedicated pools.
* [x] Currently `BroadPhase` has lots of maps, need to do something with it.
* [x] Colliders always have entries in `BroadPhase` (`BVHTree`), 
      so it makes sense to store `BroadPhaseNode(index)` inside them.
* [ ] When adding collider to child object:
  * [ ] If the user did not manually created `RigidBody` component, 'inherit' rb from parenting entity
    (if any). p.s. This should not be actually a task for physics engine.
  * [ ] If child object has dynamic `RigidBody`, 'break' local transforms 
    (objects should behave independently from each other). p.s. This should not be actually a task for physics engine.
* [x] BVH that 'minimizes' area (perimeter in 2d) instead of minimizing tree height. 
* [ ] Change `box` collider to `polygon` collider 
# Considerations:
*