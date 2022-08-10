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
	
private:
	std::shared_ptr<VertexArray> m_VAO;
	std::shared_ptr<Shader> m_Shader;
};