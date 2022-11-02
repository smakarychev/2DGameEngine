#include "RigidBodyPhysicsExample.h"

RigidBodyPhysicsExample::RigidBodyPhysicsExample() 
    : Layer("RigidBodyPhysicsExample layer"),
    m_ContactListener(CreateScope<CustomContactListener>())
{}

void RigidBodyPhysicsExample::OnAttach()
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

    m_World.SetContactListener(m_ContactListener.get());
    Physics::ContactResolver::StoreContactPoints(true);

    m_World.SetGravity(glm::vec2{ 0.0f });
    m_World.AddForce(CreateRef<Physics::Drag2D>(Physics::Drag2D(0.0f, 0.0f)));

    Physics::CircleCollider2D circleCol(glm::vec3{ 0.0f }, 0.1f);
    Physics::BoxCollider2D boxCol(glm::vec3{ 0.0f }, glm::vec2{ 0.1f, 0.1f });
    Physics::BoxCollider2D wideBoxCol(glm::vec3{ 0.0f }, glm::vec2{ 18.0f, 0.5f });
   
    Physics::ColliderDef2D colDef;
    Physics::RigidBodyDef2D rbDef{
        .Position {0.0f, 1.0f},
    };

    colDef.Collider = &wideBoxCol;
    rbDef.Position = glm::vec2{ 0.0f, -9.0f };
    rbDef.Type = Physics::RigidBodyType2D::Static;
    auto floorB = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorB, colDef);

    rbDef.Position = glm::vec2{ 0.0f, 9.0f };
    auto floorT = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorT, colDef);

    rbDef.Position = glm::vec2{ 18.0f, 0.0f };
    rbDef.Rotation = glm::radians(90.0f);
    auto floorR = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorR, colDef);

    rbDef.Position = glm::vec3{-18.0f, 0.0f, 0.0f };
    auto floorL = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorL, colDef);

    rbDef.Type = Physics::RigidBodyType2D::Dynamic;
    rbDef.Position = glm::vec2{ -10.0f, 0.0f };
    Physics::BoxCollider2D box{ glm::vec2 {0.0f}, glm::vec2{1.5f} };
    colDef.Collider = &box;
    colDef.PhysicsMaterial.Restitution = 0.0f;
    rbDef.Mass = 10.0f;
    rbDef.Inertia = rbDef.Mass / 6.0f * 4.0f * box.HalfSize.x * box.HalfSize.x;
    m_Mover = m_World.CreateBody(rbDef);
    m_World.AddCollider(m_Mover, colDef);
    glm::vec2 originalHalfSize = box.HalfSize;
    glm::vec2 originalCenter = box.Center;
    {
        box.Center += originalHalfSize;
        box.HalfSize = glm::vec2{ 0.2f };
        m_World.AddCollider(m_Mover, colDef);
        box.Center = originalCenter;
    }
    {
        box.Center -= originalHalfSize;
        box.HalfSize = glm::vec2{ 0.2f };
        m_World.AddCollider(m_Mover, colDef);
        box.Center = originalCenter;
    }

    //boulder.AddForce(glm::vec2{ 1000.0f * rbDef.Mass, 0.0f });
    //boulder.GetCollider()->SetSensor(true);
    rbDef.Type = Physics::RigidBodyType2D::Dynamic;
    rbDef.Rotation = 0.0f;
    F32 step = 0.2f / 0.25f;
    for (U32 y = 0; y < 20; y++)
    {
        for (I32 x = 0; x < 20; x++)
        {
            rbDef.Position = glm::vec2{ x * step, y * step - 10 * step };
            //rbDef.Position += glm::vec2(Random::Float2(-step * 0.5f, step * 0.5f));
            Physics::CircleCollider2D circle{ glm::vec2{0.0f}, step * 0.25f };
            Physics::BoxCollider2D box{ glm::vec2{0.0f}, glm::vec2{ 1 * step * 0.25f, step * 0.25f} };
            colDef.Collider = &box;
            colDef.PhysicsMaterial.Restitution = 0.3f;
            rbDef.Mass = 1.1f;
            rbDef.Inertia = rbDef.Mass / 6.0f * 4.0f * box.HalfSize.x * box.HalfSize.x;
            auto body = m_World.CreateBody(rbDef);
            m_World.AddCollider(body, colDef);
            //body.SetRotation(Random::Float(-Math::Pi(), Math::Pi()));
        }
    }
}

void RigidBodyPhysicsExample::OnUpdate()
{
    static F32 frameTime = 0;
    ENGINE_WARN("Frame time: {:.4f}ms", (Time::Get() - frameTime));
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    ValidateViewport();
    glm::vec2 mousePosition = m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition());
    //if (GemWarsInput::GetKey(Key::Space))
    {
        Timer timer;
        m_World.Update(dt);
        ENGINE_INFO("Simulation time: {:.4f}ms ({} fps)", timer.GetTime(), timer.GetFPS());
        if (Input::GetKey(Key::Space)) m_World.SetGravity(glm::vec2{ 0.0f, -10.0f });
        if (Input::GetKeyDown(Key::R)) m_World.SetGravity(Random::Float2(-30.0f, 30.0f));
        if (Input::GetKey(Key::A)) m_Mover->AddForce({ -10.0f,   0.0f }, Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::D)) m_Mover->AddForce({  10.0f,   0.0f }, Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::S)) m_Mover->AddForce({   0.0f, -10.0f }, Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::W)) m_Mover->AddForce({   0.0f,  10.0f }, Physics::ForceMode::Impulse);
    }
    static glm::vec2 defGrav = { 0.0f, -10.0f };
    Transform2D tr;
    tr.Rotation = glm::normalize(glm::vec2(glm::cos(Time::Get() / 3.0f), glm::sin(Time::Get() / 3.0f)));
    //m_World.SetGravity(tr.TransformDirection(defGrav));
    if (Input::GetKeyDown(Key::E)) m_World.EnableWarmStart(!m_World.IsWarmStartEnabled());
    Render();
    frameTime = F32(Time::Get());
}

void RigidBodyPhysicsExample::OnImguiUpdate()
{
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
}

void RigidBodyPhysicsExample::ValidateViewport()
{
    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    }
}

void RigidBodyPhysicsExample::Render()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera().get());
    // Render rigid bodies.
    RigidBodyWorldDrawer::Draw(m_World);

    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.Tint = {0.6f, 0.9f, 0.7f, 1.0f};
    fr.FontSize = 14.0f;
    fr.FontRect = {.Min = {(F32)m_FrameBuffer->GetSpec().Width - 150.0f, 10.0f}, .Max = {(F32)m_FrameBuffer->GetSpec().Width, 10.0f}};
    Renderer2D::DrawFontFixed(fr, "rigidbody physics example");

    static bool drawContactPoint = false;
    if (Input::GetKeyDown(Key::C)) drawContactPoint = !drawContactPoint;
    static bool drawContactNormals = false;
    if (Input::GetKeyDown(Key::N)) drawContactNormals = !drawContactNormals;

    if (drawContactPoint)
    {
        // Render contact points
        for (auto& p : Physics::ContactResolver::GetContactPoints())
        {
            Component::Transform2D transform;
            transform.Position = p;
            transform.Scale = {0.1f, 0.1f};
            Component::SpriteRenderer sp;
            sp.Tint = glm::vec4{0.65f, 0.85f, 0.35f, 1.0f};
            Renderer2D::DrawQuad(transform, sp);
        }
    }
    if (drawContactNormals)
    {
        for (U32 i = 0; i < Physics::ContactResolver::GetContactPoints().size(); i++)
        {
            glm::vec2 base = Physics::ContactResolver::GetContactPoints()[i];
            glm::vec2 dir = Physics::ContactResolver::GetContactNormals()[i];
            Renderer2D::DrawLine(base, base + dir * 0.25f);
        }
    }
    

    // Render bvh.
    static bool renderBVH = false;
    if (Input::GetKeyDown(Key::P)) 
        renderBVH = !renderBVH;
    
    if (renderBVH)
        BVHTreeDrawer::Draw(m_World.GetBroadPhase().GetBVHTree());

    Renderer2D::EndScene();

    m_FrameBuffer->Unbind();
}

CRect RigidBodyPhysicsExample::GetCameraBounds()
{
    glm::vec2 min = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, 0 });
    glm::vec2 max = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  m_ViewportSize.y });
    CRect bounds;
    bounds.Center   = { (max.x + min.x) / 2.0f, (max.y + min.y) / 2.0f };
    bounds.HalfSize = { (max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f };
    return bounds;
}
