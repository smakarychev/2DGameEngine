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
* [x] Change `box` collider to `polygon` collider 

# Considerations:
* Remove lists of colliders from body, and instead use `compoundShape`, that is basically a list (or tree)
* 2-frame cache for contact pairs.
* General architecture:  
```c++
    class PhysicsSystem
        class BodyManager
            std::vector<Body*> bodies // stores all bodies (has fixed capacity)
            std::vector<BodyId> activeBodies // stores ids of all active bodies
            
        class NarrowPhase
            class ContactGenerator
            class ContactResolver
            
        class BroadPhase
            class BVHTree // for static objects
            class BVHTree // for dynamic objects
            ...
            class BVHTree // for some objects
            
        class BodyPair
        class BodyPairManager // Creates new pair of bodies in broad phase, caches pairs
            
        class Island
            std::vector<BodyId> // bodies that belong to the island
        
        class ContactListener
        
        class ActiveListener // for active/sleep transitions
        
    class BodyId // standard index + generation pair 
    class Body // General body.
        MotionProperties* // Null if body is static
    class MotionProperties // Only non static bodies has it.
    
```
