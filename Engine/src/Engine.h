#pragma once

/* Core */
#include "Engine/Core/Core.h"

/* Application */
#include "Engine/Core/Application.h"

/* Custom types */
#include "Engine/Core/Types.h"

/* Logger */
#include "Engine/Core/Log.h"

/* Window */
#include "Engine/Core/Window.h"

/* GemWarsInput */
#include "Engine/Core/Input.h"

/* Time */
#include "Engine/Core/Time.h"

/* Custom heap allocators */
#include "Engine/Memory/StackAllocator.h"
#include "Engine/Memory/DequeAllocator.h"
#include "Engine/Memory/PoolAllocator.h"
#include "Engine/Memory/FreelistRedBlackTreeAllocator.h"
#include "Engine/Memory/BuddyAllocator.h"
#include "Engine/Memory/MemoryManager.h"

/* All math related */
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Random.h"

/* All ECS related */
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Components.h"
#include "Engine/ECS/EntityManager.h"

/* All rendering related */
#include "Engine/Rendering/Animation.h"
#include "Engine/Rendering/Buffer.h"
#include "Engine/Rendering/Font.h"
#include "Engine/Rendering/GraphicsContext.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/Renderer2D.h"
#include "Engine/Rendering/RendererAPI.h"
#include "Engine/Rendering/RenderQueue.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/SortingKey.h"
#include "Engine/Rendering/SortingLayer.h"
#include "Engine/Rendering/Texture.h"

/* Utility rendering (drawers) */
#include "Engine/Rendering/Drawers/BVHTreeDrawer.h"
#include "Engine/Rendering/Drawers/RigidBodyWorldDrawer.h"

/* UI */
#include "Engine/Imgui/ImguiLayer.h"
#include "Engine/Imgui/ImguiCommon.h"

/* All camera related */
#include "Engine/Core/Camera.h"

/* All shape related */
#include "Engine/Primitives/2D/RegularPolygon.h"

/* Common (data structures, etc). */
#include "Engine/Common/FreeList.h"
#include "Engine/Common/Geometry2D.h"
#include "Engine/Common/QuadTree.h"

/* All physics related. */
#include "Engine/Physics/ParticleEngine/Particle.h"
#include "Engine/Physics/ParticleEngine/ParticleContact.h"
#include "Engine/Physics/ParticleEngine/ParticleForceGenerator.h"
#include "Engine/Physics/ParticleEngine/ParticleLinks.h"
#include "Engine/Physics/ParticleEngine/ParticleWorld.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyForceGenerator.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BVHTree.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Collider.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Intersections.h"
#include "Engine/Physics/RigidBodyEngine/Collision/NarrowPhase.h"

/* All scene related. */
#include "Engine/Scene/Action.h"
#include "Engine/Scene/Scene.h"
