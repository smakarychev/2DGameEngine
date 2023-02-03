#include "RigidBodyNewPhysicsExample.h"

RigidBodyNewPhysicsExample::RigidBodyNewPhysicsExample() 
    : Layer("RigidBodyNewPhysicsExample layer"),
    m_ContactListener(CreateScope<WCustomContactListener>())
{}

void RigidBodyNewPhysicsExample::OnAttach()
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

    m_World.SetContactListener(m_ContactListener.Get());
    WIP::Physics::ContactResolver::StoreContactPoints(true);

    m_World.SetGravity(glm::vec2{ 0.0f });

    WIP::Physics::PolygonCollider2D boxCol;
    boxCol.SetAsBox({0.1f, 0.1f});
    WIP::Physics::PolygonCollider2D wideBoxCol;
    wideBoxCol.SetAsBox(glm::vec2{ 18.0f, 2.5f });
    Component::LocalToWorldTransform2D tf;
    tf.Position = {0.0f, 1.0f};
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    WIP::Physics::ColliderDef2D colDef;
    WIP::Physics::RigidBodyDef2D rbDef{
        .Transform = *m_Transforms.back()
    };
    colDef.Collider = &wideBoxCol;
    tf.Position = { 0.0f, -9.0f };
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    rbDef.Type = WIP::Physics::RigidBodyType2D::Static;
    auto floorB = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorB, colDef);

    tf.Position = { 0.0f, 9.0f };
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    auto floorT = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorT, colDef);

    tf.Position = { 18.0f, 0.0f };
    tf.Rotation = glm::radians(90.0f);
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    auto floorR = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorR, colDef);

    tf.Position = { -18.0f, 0.0f };
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    auto floorL = m_World.CreateBody(rbDef);
    m_World.AddCollider(floorL, colDef);

    rbDef.Type = WIP::Physics::RigidBodyType2D::Dynamic;
    tf.Position = { -10.0f, 0.0f };
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    WIP::Physics::PolygonCollider2D somethingWicked(
        {{
            {-2.0f,  0.0f},
            { 0.0f, -0.3f},
            { 0.0f,  0.3f},
            { 2.0f,  0.0f},
        }}
    );
    //somethingWicked.SetAsBox(glm::vec2{1.5f});
    tf.Rotation = 0.0;
    m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
    rbDef.Transform = *m_Transforms.back();
    colDef.Collider = &somethingWicked;
    colDef.PhysicsMaterial.Restitution = 0.0f;
    colDef.PhysicsMaterial.Density = 10.0f;
    m_Mover = m_World.CreateBody(rbDef);
    m_World.AddCollider(m_Mover, colDef);
    WIP::Physics::PolygonCollider2D somethingWicked2(
        {{
            { 0.0f, -2.0f},
            { -0.3f, 0.0f},
            { 0.3f,  0.0f},
            { 0.0f,  2.0f},
        }}
    );
    colDef.Collider = &somethingWicked2;
    m_World.AddCollider(m_Mover, colDef);
    //m_Mover->AddForce(glm::vec2{ 2000.0f * m_Mover->GetMass(), 0.0f });

    //boulder.AddForce(glm::vec2{ 1000.0f * rbDef.Mass, 0.0f });
    //boulder.GetCollider()->SetSensor(true);
    rbDef.Type = WIP::Physics::RigidBodyType2D::Dynamic;
    tf.Rotation = 0.0;
    F32 step = 0.1f / 0.25f;
    WIP::Physics::PolygonCollider2D hexagon(
        {{
            { 0.0f, -0.2f},
            { 0.2f, -0.1f},
            { 0.2f,  0.1f},
            { 0.0f,  0.2f},
            {-0.2f, -0.1f},
            {-0.2f,  0.1f},
        }}
    );
    
    for (U32 y = 0; y < 21; y++)
    {
        for (I32 x = 0; x < 40; x++)
        {
            tf.Position = { x * step - 5 * step, y * step - 10.5f * step };
            tf.Rotation = Random::Float(-Math::Pi(), Math::Pi());
            m_Transforms.push_back(CreateRef<Component::LocalToWorldTransform2D>(tf));
            rbDef.Transform = *m_Transforms.back();
            //rbDef.Position += glm::vec2(Random::Float2(-step * 0.5f, step * 0.5f));
            WIP::Physics::PolygonCollider2D box;
            auto bbox = box.SetAsBox(glm::vec2{ step * 0.5f, step * 0.5f});
            colDef.Collider = &hexagon;
            colDef.PhysicsMaterial.Density = 1.0f;
            colDef.PhysicsMaterial.Restitution = 0.3f;
            auto body = m_World.CreateBody(rbDef);
            m_World.AddCollider(body, colDef);
            //body.SetRotation(Random::Float(-Math::Pi(), Math::Pi()));
        }
    }
    
    ENGINE_CORE_INFO("Tree cost: {}", m_World.GetBroadPhase().GetBVHTree().ComputeTotalCost());
}

void RigidBodyNewPhysicsExample::OnUpdate()
{
    static F32 frameTime = 0;
    static F32 worstFrameTime = 0;
    static F32 avgFrameTime = 0;
    static F32 alphaFilter = 0.2f;
    static U32 frames = 1;
    ENGINE_WARN("Frame time: {:.4f}ms", (Time::Get() - frameTime));
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    ValidateViewport();
    glm::vec2 mousePosition = m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition());
    //if (GemWarsInput::GetKey(Key::Space))
    {
        Timer timer;
        m_World.Update(dt);
        F32 dt = timer.GetTime();
        if (dt > worstFrameTime) worstFrameTime += alphaFilter * (dt - worstFrameTime);
        avgFrameTime = avgFrameTime + 1.0f / frames * (dt - avgFrameTime);
        ENGINE_INFO("Simulation time: {:.4f}ms, Worst: {:.4f}ms, Avg: {:.4f}ms", dt, worstFrameTime, avgFrameTime);
        if (Input::GetKey(Key::Space)) m_World.SetGravity(glm::vec2{ 0.0f, -10.0f });
        if (Input::GetKey(Key::X)) m_World.SetGravity(glm::vec2{ 0.0f, 0.0f });
        if (Input::GetKeyDown(Key::R)) m_World.SetGravity(Random::Float2(-30.0f, 30.0f));
        if (Input::GetKey(Key::A)) m_Mover->AddForce({ -10.0f,   0.0f }, WIP::Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::D)) m_Mover->AddForce({  10.0f,   0.0f }, WIP::Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::S)) m_Mover->AddForce({   0.0f, -10.0f }, WIP::Physics::ForceMode::Impulse);
        if (Input::GetKey(Key::W)) m_Mover->AddForce({   0.0f,  10.0f }, WIP::Physics::ForceMode::Impulse);
    }
    static glm::vec2 defGrav = { 0.0f, -10.0f };
    //m_World.SetGravity(tr.TransformDirection(defGrav));
    if (Input::GetKeyDown(Key::E)) m_World.EnableWarmStart(!m_World.IsWarmStartEnabled());
    Render();
    frameTime = F32(Time::Get());
    frames++;
    if (frames > 300)
    {
        avgFrameTime = 0.0f; frames = 1;
    }
    ENGINE_INFO("Tree cost: {}", m_World.GetBroadPhase().GetBVHTree().ComputeTotalCost());
}

void RigidBodyNewPhysicsExample::OnImguiUpdate()
{
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
}

void RigidBodyNewPhysicsExample::ValidateViewport()
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

void RigidBodyNewPhysicsExample::Render()
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
        for (auto& p : WIP::Physics::ContactResolver::GetContactPoints())
        {
            Component::LocalToWorldTransform2D transform;
            transform.Position = p;
            transform.Scale = {0.1f, 0.1f};
            Component::SpriteRenderer sp;
            sp.Tint = glm::vec4{0.65f, 0.85f, 0.35f, 1.0f};
            Renderer2D::DrawQuad(transform, sp);
        }
    }
    if (drawContactNormals)
    {
        for (U32 i = 0; i < WIP::Physics::ContactResolver::GetContactPoints().size(); i++)
        {
            glm::vec2 base = WIP::Physics::ContactResolver::GetContactPoints()[i];
            glm::vec2 dir = WIP::Physics::ContactResolver::GetContactNormals()[i];
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

CRect RigidBodyNewPhysicsExample::GetCameraBounds()
{
    glm::vec2 min = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, 0 });
    glm::vec2 max = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  m_ViewportSize.y });
    CRect bounds;
    bounds.Center   = { (max.x + min.x) / 2.0f, (max.y + min.y) / 2.0f };
    bounds.HalfSize = { (max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f };
    return bounds;
}
