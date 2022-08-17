#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

class GameLayer : public Layer
{
public:
	GameLayer() : Layer("Game layer")
	{}
	
	void OnAttach() override;
	void OnUpdate() override;
	void OnEvent(Event& event) override;
	
private:
	std::shared_ptr<VertexArray> m_VAO;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_Texture;
	std::shared_ptr<Texture> m_Brick;
	std::shared_ptr<Texture> m_Tree;
	std::shared_ptr<Texture> m_Tile;
	std::shared_ptr<Texture> m_Tileset;
	std::shared_ptr<CameraController> m_CameraController;
};