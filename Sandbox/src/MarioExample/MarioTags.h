#pragma once

#include "Engine.h"

using namespace Engine::Types;
using namespace Engine;

// NOTE: 'Mario' prefix here is not indicating that this component belongs to the Mario himself,
// it is merely showing that it is a part of the Mario game.

CREATE_TAG_COMPONENT(MarioPlayerTag)
CREATE_TAG_COMPONENT(MarioLevelTag)
CREATE_TAG_COMPONENT(MarioEmptyBlockTag)
CREATE_TAG_COMPONENT(MarioCoinBlockTag)
CREATE_TAG_COMPONENT(MarioCoinTag)

CREATE_TAG_COMPONENT(MarioEnemyTag)
CREATE_TAG_COMPONENT(MarioGoombaTag)
CREATE_TAG_COMPONENT(MarioPiranhaPlantTag)
CREATE_TAG_COMPONENT(MarioKoopaTag)

// To 'awake' enemies when player is near.
CREATE_TAG_COMPONENT(MarioAwakeTag)
// To kill player and enemies if they are outside of screen bounds.
CREATE_TAG_COMPONENT(MarioKillTag)
// To win the game.
CREATE_TAG_COMPONENT(MarioWinTag)
// When player/enemies are on death door or in some other conditions that makes them `harmless` (incapable of dealing damage).
CREATE_TAG_COMPONENT(MarioHarmlessTag)
