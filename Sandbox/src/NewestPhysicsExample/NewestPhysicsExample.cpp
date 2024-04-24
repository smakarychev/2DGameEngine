#include "NewestPhysicsExample.h"

#include "Engine/Physics/NewRBE/Newest/PhysicsFactory.h"
#include "Engine/Physics/NewRBE/Newest/Collision/Colliders/PolygonCollider2D.h"

NewestPhysicsExample::NewestPhysicsExample()
    : Layer("NewestPhysics")
{
}

void NewestPhysicsExample::OnAttach()
{
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);
    camera->SetZoom(10.0f);
    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);
    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");

    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
    m_FrameBuffer = FrameBuffer::Create(spec);
    RenderCommand::SetClearColor(glm::vec3{ 0.1f, 0.1f, 0.1f });

    WIP::Physics::Newest::PhysicsFactory::Init();
    m_PhysicsSystem.Init(1024, CreateRef<CustomBroadPhaseLayers>(), CreateRef<CustomBodyToBroadPhaseLayerFilter>());
    auto& bodyManager = m_PhysicsSystem.GetBodyManager();
    WIP::Physics::Newest::DynamicsDataDesc2D ddDesc{};
    WIP::Physics::Newest::RigidBodyDesc2D rbDesc{};
    rbDesc.BodyType = WIP::Physics::Newest::BodyType::Dynamic;
    rbDesc.CollisionLayer = Moving;
    rbDesc.DynamicsDataDesc = ddDesc;
    auto* moving = bodyManager.CreateBody(rbDesc);
    bodyManager.AddBody(moving, WIP::Physics::Newest::StartUpBehaviour::SetActive);
    auto* box = reinterpret_cast<WIP::Physics::Newest::PolygonCollider2D*>(bodyManager.SetCollider(moving->GetId(), WIP::Physics::Newest::PolygonColliderDesc2D{}));
    box->SetAsBox({0.2f, 0.3f});

    rbDesc.BodyType = WIP::Physics::Newest::BodyType::Static;
    rbDesc.CollisionLayer = NonMoving;
    rbDesc.Transform.Position = {0.0f, -5.0f};
    auto* nonMoving = bodyManager.CreateBody(rbDesc);
    bodyManager.AddBody(nonMoving, WIP::Physics::Newest::StartUpBehaviour::SetInactive);
    auto* wideBox = reinterpret_cast<WIP::Physics::Newest::PolygonCollider2D*>(bodyManager.SetCollider(nonMoving->GetId(), WIP::Physics::Newest::PolygonColliderDesc2D{}));
    wideBox->SetAsBox({10.0f, 5.0f});
    m_PhysicsSystem.UpdateBodyCollider(nonMoving->GetId());
}

void NewestPhysicsExample::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    m_PhysicsSystem.Update(dt);
    auto& activeBodies = m_PhysicsSystem.GetBodyManager().GetActiveBodies();
    auto& bodies = m_PhysicsSystem.GetBodyManager().GetBodies();
    auto& bp = m_PhysicsSystem.GetBroadPhase();
    for (auto& ab : activeBodies)
    {
        ENGINE_INFO("Body: {}, Pos: [{:.3f} {:.3f}], Vel: [{:.3f} {:.3f}]",
            ab,
            bodies[ab]->GetPosition().x, bodies[ab]->GetPosition().y,
            bodies[ab]->GetLinearVelocity().x, bodies[ab]->GetLinearVelocity().y);
    }
    ValidateViewport();
    Render();
}

void NewestPhysicsExample::Render()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera().get());

    auto& bvhTrees = m_PhysicsSystem.GetBroadPhase().GetTrees();
    for (auto& tree : bvhTrees) BVHTreeDrawer::Draw(tree);
    RigidBodyWorldDrawer::Draw(m_PhysicsSystem);
    
    Renderer2D::EndScene();
    m_FrameBuffer->Unbind();
}

void NewestPhysicsExample::OnImguiUpdate()
{
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
}

void NewestPhysicsExample::OnDetach()
{
    m_PhysicsSystem.ShutDown();
    WIP::Physics::Newest::PhysicsFactory::ShutDown();
}

void NewestPhysicsExample::ValidateViewport()
{
    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != (U32)m_ViewportSize.x || spec.Height != (U32)m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    }
}

