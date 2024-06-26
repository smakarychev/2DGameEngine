#pragma once

#include <Engine.h>

#include "Engine/ECS/Registry.h"

using namespace Engine;
using namespace Engine::Types;

struct AABB
{
	glm::vec2 BottomLeft;
	glm::vec2 TopRight;
};

class GemWarsExample : public Layer
{
public:
	GemWarsExample() : Layer("Game layer")
	{}
	
	void OnAttach() override;
	void OnUpdate() override;
	void OnImguiUpdate() override;
	void OnEvent(Event& event) override;
private:
	void SetBounds();
	void SetPaused(bool isPaused) { m_IsRunning = !isPaused; }
	void SpawnPlayer();
	void SpawnParticles(Entity entity);
	void SpawnBullet(Entity entity, const glm::vec2& target);

	void sEnemySpawner();
	void sMovement(F32 dt);
	void sCollision();
	void sUserInput();
	void sRender();
	void sParticleUpdate();
	void sSpecialAbility();
	void sAddScore(Entity entity);

	bool Collide(Entity a, Entity b);

	bool OnWindowResize(WindowResizeEvent& event);
private:
	Ref<Texture> m_Tileset;
	Ref<Texture> m_Background;
	Ref<CameraController> m_CameraController;

	Registry m_Registry;
	
	U64 m_CurrentFrame = 0;
	U64 m_LastEnemySpawnTime = 0;
	U64 m_IsRunning = true;

	Entity m_Player;

	AABB m_Bounds = {};

	Ref<Font> m_Font;

	Ref<FrameBuffer> m_FrameBuffer;
	glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };
};