#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

struct AABB
{
	glm::vec2 BottomLeft;
	glm::vec2 TopRight;
};

class GameLayer : public Layer
{
public:
	GameLayer() : Layer("Game layer")
	{}
	
	void OnAttach() override;
	void OnUpdate() override;
	void OnEvent(Event& event) override;

private:
	void SetBounds();
	void SetPaused(bool isPaused) { m_IsRunning = !isPaused; }
	void SpawnPlayer();
	void SpawnParticles(Entity& entity);
	void SpawnBullet(Entity& entity, const glm::vec2& target);

	void sEnemySpawner();
	void sMovement(F32 dt);
	void sCollision();
	void sUserInput();
	void sRender();
	void sParticleUpdate();
	void sSpecialAbility();

	bool Collide(Entity& a, Entity& b);

private:
	std::shared_ptr<Texture> m_Tileset;
	std::shared_ptr<Texture> m_Background;
	std::shared_ptr<CameraController> m_CameraController;

	EntityManager m_Manager;
	U64 m_CurrentFrame = 0;
	U64 m_LastEnemySpawnTime = 0;
	U64 m_IsRunning = true;

	Entity* m_Player;

	AABB m_Bounds;
};