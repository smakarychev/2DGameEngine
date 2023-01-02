#pragma once

/* Core */
#include "Engine/Core/Application.h"
#include "Engine/Core/Camera.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/UUID.h"
#include "Engine/Core/Window.h"

/* Common */
#include "Engine/Common/FreeList.h"
#include "Engine/Common/Geometry2D.h"
#include "Engine/Common/QuadTree.h"
#include "Engine/Common/SparseSet.h"
#include "Engine/Common/SparseSetPaged.h"

/* All ECS related */
#include "Engine/ECS/Components.h"
#include "Engine/ECS/ComponentsManager.h"
#include "Engine/ECS/EntityId.h"
#include "Engine/ECS/EntityManager.h"
#include "Engine/ECS/Registry.h"
#include "Engine/ECS/View.h"

/* Events */
#include "Engine/Events/Event.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"
#include "Engine/Events/WindowEvents.h"

/* UI */
#include "Engine/Imgui/ImguiCommon.h"
#include "Engine/Imgui/ImguiLayer.h"

/* All math related */
#include "Engine/Math/LinearAlgebra.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Random.h"

/* Custom heap allocators */
#include "Engine/Memory/BuddyAllocator.h"
#include "Engine/Memory/DequeAllocator.h"
#include "Engine/Memory/FreelistRedBlackTreeAllocator.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Memory/MemoryUtils.h"
#include "Engine/Memory/PoolAllocator.h"
#include "Engine/Memory/StackAllocator.h"
#include "Engine/Memory/Handle/Handle.h"

/* All physics related */
#include "Engine/Physics/ParticleEngine/Particle.h"
#include "Engine/Physics/ParticleEngine/ParticleContact.h"
#include "Engine/Physics/ParticleEngine/ParticleForceGenerator.h"
#include "Engine/Physics/ParticleEngine/ParticleLinks.h"
#include "Engine/Physics/ParticleEngine/ParticleWorld.h"
#include "Engine/Physics/RigidBodyEngine/PhysicsMaterial.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyForceGenerator.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BVHTree.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Collider.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Intersections.h"
#include "Engine/Physics/RigidBodyEngine/Collision/NarrowPhase.h"

/* All shape related */
#include "Engine/Primitives/2D/RegularPolygon.h"

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

/* Resource management */
#include "Engine/Resource/ResourceManager.h"

/* All scene related. */
#include "Engine/Scene/Action.h"
#include "Engine/Scene/ComponentUI.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneGraph.h"
#include "Engine/Scene/ScenePanels.h"
#include "Engine/Scene/SceneUtils.h"

/* All scene-serialization related. */
#include "Engine/Scene/Serialization/ComponentSerializer.h"
#include "Engine/Scene/Serialization/Prefab.h"
#include "Engine/Scene/Serialization/SceneSerializer.h"
#include "Engine/Scene/Serialization/SerializationUtils.h"