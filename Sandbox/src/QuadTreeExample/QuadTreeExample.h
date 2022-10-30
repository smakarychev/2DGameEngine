#pragma once

#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

struct ColoredQuad
{
	glm::vec3 pos;
	glm::vec2 vel;
	glm::vec2 size;
	glm::vec4 color;
};

class QuadTreeExample : public Layer
{
public:
	QuadTreeExample() : Layer("QuadTree layer")
	{}
	void OnAttach() override;
	void OnUpdate() override;
	void OnImguiUpdate() override;
private:
	void PopulateQuadTree();
	void Render();
	CRect GetCameraBounds();
private:
	Ref<CameraController> m_CameraController;
	QuadTreeContainer<ColoredQuad> m_QuadTree;
	U32 m_MaxQuads = 0;

	Ref<Font> m_Font;

	Ref<FrameBuffer> m_FrameBuffer;
	glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };

};