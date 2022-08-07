#pragma once

/* Core */
#include "Engine/Core/Core.h"

/* Application */
#include "Engine/Core/Application.h"

/* Entry point */
#include "Engine/Core/EntryPoint.h"

/* Custom types */
#include "Engine/Core/Types.h"

/* Logger */
#include "Engine/Core/Log.h"

/* Window */
#include "Engine/Core/Window.h"

/* Input */
#include "Engine/Core/Input.h"

/* Custom heap allocators */
#include "Engine/Memory/StackAllocator.h"
#include "Engine/Memory/DequeAllocator.h"
#include "Engine/Memory/PoolAllocator.h"
#include "Engine/Memory/FreelistRedBlackTreeAllocator.h"
#include "Engine/Memory/BuddyAllocator.h"
#include "Engine/Memory/MemoryManager.h"

/* All ECS related */
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Components.h"
#include "Engine/ECS/EntityManager.h"