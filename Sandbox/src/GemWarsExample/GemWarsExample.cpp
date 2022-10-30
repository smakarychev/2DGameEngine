#include "GemWarsExample.h"

void GemWarsExample::OnAttach()
{
    m_Tileset = Texture::LoadTextureFromFile("assets/textures/cavesofgallet_tiles.png");
    m_Background = m_Tileset->GetSubTexture({ 8.0f, 8.0f }, { 3, 11 });
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);

    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);
    camera->SetZoom(3.0f);

    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));

    SetBounds();

    SpawnPlayer();
    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");
    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
    m_FrameBuffer = FrameBuffer::Create(spec);
}

void GemWarsExample::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);

    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        SetBounds();
    }

    sUserInput();
    if (m_IsRunning)
    {
        m_Manager.Update();
        sMovement(dt);
        sCollision();
        sEnemySpawner();
        sParticleUpdate();
        sSpecialAbility();
        // Let's hope player doesn't play for more than 35 trillion years.
        m_CurrentFrame++;
    }
    sRender();
}

void GemWarsExample::OnImguiUpdate()
{  
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportSize.x, viewportSize.y };
    U64 textureID = m_FrameBuffer->GetColorBufferId(0);
    ImGui::Image(reinterpret_cast<void*>(textureID), viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    ImGui::PopStyleVar(1);
    ImGui::End();
}


void GemWarsExample::SpawnPlayer()
{
    Entity* entity = &m_Manager.AddEntity("player");
    F32 playerRadius = 0.2f;
    entity->GemWarsTransform2D = CreateRef<Component::GemWarsTransform2D>(glm::vec2{ 0.0f, 0.0f }, glm::vec2{ playerRadius }, 0.0f);
    entity->GemWarsRigidBody2D = CreateRef<Component::GemWarsRigidBody2D>(playerRadius, 2.0f);
    entity->GemWarsMesh2D = CreateRef<Component::Mesh2D>(8, nullptr, glm::vec4{ 0.78f, 0.55f, 0.16f, 1.0f });
    entity->GemWarsInput = CreateRef<Component::GemWarsInput>();
    entity->GemWarsSpecialAbility = CreateRef<Component::GemWarsSpecialAbility>(120);
    entity->GemWarsScore = CreateRef<Component::GemWarsScore>(0);
    m_Player = entity;
    for (auto& e : m_Manager.GetEntities("enemy"))
    {
        e->Destroy();
    }
    for (auto& e : m_Manager.GetEntities("particle"))
    {
        e->Destroy();
    }
}

void GemWarsExample::sEnemySpawner()
{
    I32 spawnThreshold = 2 * 60;
    if (m_CurrentFrame - m_LastEnemySpawnTime > spawnThreshold)
    {
        U32 enemiesToSpawn = Random::UInt(1, 3);
        for (U32 i = 0; i < enemiesToSpawn; i++)
        {
            Entity& enemy = m_Manager.AddEntity("enemy");
            F32 enemyRadius = Random::Float(0.15f, 0.45f);
            glm::vec2 allowedXRegion = glm::vec2{ m_Bounds.BottomLeft.x + enemyRadius, m_Bounds.TopRight.x - enemyRadius };
            glm::vec2 allowedYRegion = glm::vec2{ m_Bounds.BottomLeft.y + enemyRadius, m_Bounds.TopRight.y - enemyRadius };
            glm::vec2 enemyPosition = glm::vec2(Random::Float(allowedXRegion.x, allowedXRegion.y), Random::Float(allowedYRegion.x, allowedYRegion.y));
            enemy.GemWarsTransform2D = CreateRef<Component::GemWarsTransform2D>(enemyPosition, glm::vec2{ enemyRadius }, 0.0f);
            enemy.GemWarsRigidBody2D = CreateRef<Component::GemWarsRigidBody2D>(enemyRadius, Random::Float(2.0, 5.0f), Random::Float(glm::radians(30.0f), glm::radians(90.0f)));
            while (Collide(enemy, *m_Player))
            {
                enemy.GemWarsTransform2D->Position = glm::vec3(Random::Float(allowedXRegion.x, allowedXRegion.y), Random::Float(allowedYRegion.x, allowedYRegion.y), 0.0f);
            }

            enemy.GemWarsRigidBody2D->Velocity = glm::normalize(Random::Float2(-1.0, 1.0));
            enemy.GemWarsMesh2D = CreateRef<Component::Mesh2D>(Random::UInt(3, 8), nullptr, glm::vec4(Random::Float3(0.2f, 0.6f), 1.0));
            enemy.GemWarsScore = CreateRef<Component::GemWarsScore>(enemy.GemWarsMesh2D->Shape.GetNumberOfVertices() * 10);
        }
       
        m_LastEnemySpawnTime = m_CurrentFrame;
    }
}

void GemWarsExample::SpawnParticles(Entity& entity)
{
    U32 numberOfParticles = entity.GemWarsMesh2D->Shape.GetNumberOfVertices();
    glm::vec2 particlesSize = entity.GemWarsTransform2D->Scale / 2.0f;
    F32 particleSpeed = 2.0f;
    I32 particlesLifetime = 60;

    for (U32 i = 0; i < numberOfParticles; i++)
    {
        F32 phase = 2.0f * glm::pi<float>() * i / numberOfParticles;
        F32 rotation = entity.GemWarsTransform2D->Rotation;
        glm::vec3 particlePos = glm::vec3(glm::vec2(glm::sin(phase), glm::cos(phase)), 0.0f) * entity.GemWarsRigidBody2D->CollisionRadius;
        particlePos = {
            glm::cos(rotation) * particlePos.x - glm::sin(rotation) * particlePos.y,
            glm::sin(rotation) * particlePos.x + glm::cos(rotation) * particlePos.y,
            0.0f
        };
        particlePos += entity.GemWarsTransform2D->Position;
        glm::vec2 particleVelocity = glm::normalize(particlePos - entity.GemWarsTransform2D->Position);

        Entity& particle = m_Manager.AddEntity("particle");
        particle.GemWarsTransform2D = CreateRef<Component::GemWarsTransform2D>(particlePos, particlesSize, 0.0f);
        particle.GemWarsRigidBody2D = CreateRef<Component::GemWarsRigidBody2D>(particlesSize.x, particleSpeed);
        particle.GemWarsRigidBody2D->Velocity = particleVelocity;
        particle.GemWarsMesh2D = CreateRef<Component::Mesh2D>(numberOfParticles, nullptr, entity.GemWarsMesh2D->Tint);
        particle.GemWarsLifeSpan = CreateRef<Component::GemWarsLifeSpan>(particlesLifetime);
    }

}

void GemWarsExample::SpawnBullet(Entity& entity, const glm::vec2& target)
{
    F32 bulletSpeed = 15.0f;
    F32 bulletRadius = 0.02f;
    glm::vec2 bulletVelocity = target - glm::vec2(entity.GemWarsTransform2D->Position);
    bulletVelocity += Random::Float2(-0.3f, 0.3f);
    bulletVelocity = glm::normalize(bulletVelocity);
    glm::vec2 bulletPosition = glm::vec2(entity.GemWarsTransform2D->Position) + entity.GemWarsRigidBody2D->CollisionRadius * bulletVelocity;
    Entity& bullet = m_Manager.AddEntity("bullet");
    bullet.GemWarsTransform2D = CreateRef<Component::GemWarsTransform2D>(bulletPosition, glm::vec2{ bulletRadius }, 0.0f);
    bullet.GemWarsRigidBody2D = CreateRef<Component::GemWarsRigidBody2D>(bulletRadius, bulletSpeed);
    bullet.GemWarsRigidBody2D->Velocity = bulletVelocity;
    bullet.GemWarsMesh2D = CreateRef<Component::Mesh2D>(4, nullptr, glm::vec4(0.6f, 0.9f, 0.2f, 1.0f));
}

void GemWarsExample::sSpecialAbility()
{
    static I32 cloud = 150;
    if (m_Player->GemWarsInput->SpecialAbility)
    {
        if (m_Player->GemWarsSpecialAbility->RemainingCoolDown == 0)
        {
            for (I32 i = 0; i < cloud; i++)
            {
                F32 phase = float(i) / cloud * 2 * glm::pi<F32>();
                glm::vec2 position = { glm::sin(phase) * m_Player->GemWarsRigidBody2D->CollisionRadius, glm::cos(phase) * m_Player->GemWarsRigidBody2D->CollisionRadius };
                position += glm::vec2(m_Player->GemWarsTransform2D->Position);
                SpawnBullet(*m_Player, position);
            }
            m_Player->GemWarsSpecialAbility->RemainingCoolDown = m_Player->GemWarsSpecialAbility->CoolDown;
        }
    }
    if (m_Player->GemWarsSpecialAbility->RemainingCoolDown > 0) m_Player->GemWarsSpecialAbility->RemainingCoolDown--;

}

void GemWarsExample::sAddScore(Entity& entity)
{
    m_Player->GemWarsScore->TotalScore += entity.GemWarsScore->TotalScore;
    entity.GemWarsScore->TotalScore = 0;
}

void GemWarsExample::sMovement(F32 dt)
{
    // Update player.
    glm::vec2 velocity = glm::vec2(0.0f);
    if (m_Player->GemWarsInput->Left)  velocity.x -= 1.0f;
    if (m_Player->GemWarsInput->Right) velocity.x += 1.0f;
    if (m_Player->GemWarsInput->Up)    velocity.y += 1.0f;
    if (m_Player->GemWarsInput->Down)  velocity.y -= 1.0f;
    if (velocity != glm::vec2(0.0f)) velocity = glm::normalize(velocity);
    m_Player->GemWarsRigidBody2D->Velocity = velocity;

    if (m_Player->GemWarsInput->Shoot) SpawnBullet(*m_Player, m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition()));
    // Update all.
    for (auto& entity : m_Manager.GetEntities())
    {
        entity->GemWarsTransform2D->Position += glm::vec3(entity->GemWarsRigidBody2D->Velocity * entity->GemWarsRigidBody2D->Speed * dt, 0.0f);
        entity->GemWarsTransform2D->Rotation += entity->GemWarsRigidBody2D->RotationSpeed * dt;
    }
       
}

void GemWarsExample::sCollision()
{
    // Check for player-enemy collision.
    for (auto& entity : m_Manager.GetEntities("enemy"))
    {
        if (Collide(*m_Player, *entity))
        {
            m_Player->Destroy();
            SpawnPlayer();
            SetPaused(m_IsRunning);
        }
    }
    for (auto& entity : m_Manager.GetEntities("particle"))
    {
        if (Collide(*m_Player, *entity))
        {
            m_Player->Destroy();
            SpawnPlayer();
            SetPaused(m_IsRunning);
        }
    }

    // Check for player-walls collision.
    F32 playerRadius = m_Player->GemWarsRigidBody2D->CollisionRadius;
    glm::vec3& playerPosition = m_Player->GemWarsTransform2D->Position;
    if (playerPosition.x + playerRadius > m_Bounds.TopRight.x) playerPosition.x = m_Bounds.TopRight.x - playerRadius;
    if (playerPosition.y + playerRadius > m_Bounds.TopRight.y) playerPosition.y = m_Bounds.TopRight.y - playerRadius;
    if (playerPosition.x - playerRadius < m_Bounds.BottomLeft.x) playerPosition.x = m_Bounds.BottomLeft.x + playerRadius;
    if (playerPosition.y - playerRadius < m_Bounds.BottomLeft.y) playerPosition.y = m_Bounds.BottomLeft.y + playerRadius;
    
    // Check for enemy-walls collision.
    for (auto& entity : m_Manager.GetEntities("enemy"))
    {
        if (entity->GemWarsTransform2D->Position.x + entity->GemWarsRigidBody2D->CollisionRadius > m_Bounds.TopRight.x ||
            entity->GemWarsTransform2D->Position.x - entity->GemWarsRigidBody2D->CollisionRadius < m_Bounds.BottomLeft.x)
        {
            entity->GemWarsRigidBody2D->Velocity.x *= -1.0f;
        }
        if (entity->GemWarsTransform2D->Position.y + entity->GemWarsRigidBody2D->CollisionRadius > m_Bounds.TopRight.y ||
            entity->GemWarsTransform2D->Position.y - entity->GemWarsRigidBody2D->CollisionRadius < m_Bounds.BottomLeft.y)
        {
            entity->GemWarsRigidBody2D->Velocity.y *= -1.0f;
        }
    }
    
    // Check bullet-enemy collision.
    for (auto& bullet : m_Manager.GetEntities("bullet"))
    {
        for (auto& enemy : m_Manager.GetEntities("enemy"))
        {
            if (Collide(*bullet, *enemy))
            {
                bullet->Destroy(); enemy->Destroy();
                SpawnParticles(*enemy);
                sAddScore(*enemy);
            }
        }
    }

    // Check bullet-wall collision.
    for (auto& bullet : m_Manager.GetEntities("bullet"))
    {
        if (bullet->GemWarsTransform2D->Position.x - bullet->GemWarsRigidBody2D->CollisionRadius > m_Bounds.TopRight.x ||
            bullet->GemWarsTransform2D->Position.x + bullet->GemWarsRigidBody2D->CollisionRadius < m_Bounds.BottomLeft.x)
        {
            bullet->Destroy();
        }
        if (bullet->GemWarsTransform2D->Position.y - bullet->GemWarsRigidBody2D->CollisionRadius > m_Bounds.TopRight.y ||
            bullet->GemWarsTransform2D->Position.y + bullet->GemWarsRigidBody2D->CollisionRadius < m_Bounds.BottomLeft.y)
        {
            bullet->Destroy();
        }
    }
}

void GemWarsExample::sUserInput()
{
    m_Player->GemWarsInput->Left = Input::GetKey(Key::A);
    m_Player->GemWarsInput->Right = Input::GetKey(Key::D);
    m_Player->GemWarsInput->Up = Input::GetKey(Key::W);
    m_Player->GemWarsInput->Down = Input::GetKey(Key::S);
    m_Player->GemWarsInput->Shoot = Input::GetMouseButton(Mouse::Button0);
    m_Player->GemWarsInput->SpecialAbility = Input::GetMouseButton(Mouse::Button1);

    if (Input::GetKeyDown(Key::P)) SetPaused(m_IsRunning);
}

void GemWarsExample::sRender()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera());

    Component::Transform2D transform;
    transform.Scale = {40.0f, 40.0f};
    Component::SpriteRenderer sp;
    sp.Texture = m_Background.get();
    sp.Tiling = {20.0f, 20.0f};
    sp.OrderInLayer = -1;
    Renderer2D::DrawQuad(transform, sp);

    for (auto& entity : m_Manager.GetEntities())
    {
        if (!entity->GemWarsMesh2D) continue;
        Component::Transform2D transform;
        transform.Position = entity->GemWarsTransform2D->Position;
        transform.Scale = entity->GemWarsTransform2D->Scale;
        transform.Rotation = entity->GemWarsTransform2D->Rotation;
        Component::PolygonRenderer pr;
        pr.Polygon = &entity->GemWarsMesh2D->Shape;
        pr.Tint = entity->GemWarsMesh2D->Tint;
        pr.OrderInLayer = 1;
        Renderer2D::DrawPolygon(transform, pr);
    }
    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.Tint = {0.6f, 0.9f, 0.7f, 1.0f};
    fr.FontSize = 36.0f;
    fr.FontRect = {.Min = {10.0f, 10.0f}, .Max = {1600.0f, 10.0f}};
    Renderer2D::DrawFontFixed(fr, std::format("score: {:d}", m_Player->GemWarsScore->TotalScore));
    fr.FontSize = 14.0f;
    fr.FontRect = {.Min = {(F32)m_FrameBuffer->GetSpec().Width - 90.0f, 10.0f}, .Max = {(F32)m_FrameBuffer->GetSpec().Width, 10.0f}};
    Renderer2D::DrawFontFixed(fr,"GemWars example");

    Renderer2D::EndScene();

    m_FrameBuffer->Unbind();
    /* RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera());
    F32 aspect = (F32)m_CameraController->GetCamera()->GetViewportWidth() / (F32)m_CameraController->GetCamera()->GetViewportHeight();
    Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 6.0f * aspect, 6.0f }, m_FrameBuffer->GetColorBuffer(), {1, 1});
    Renderer2D::EndScene();*/
}

void GemWarsExample::sParticleUpdate()
{
    for (auto& particle : m_Manager.GetEntities("particle"))
    {
        I32 lifeSpan = particle->GemWarsLifeSpan->Remaining;
        if (lifeSpan <= 0) particle->Destroy();
        else
        {
            I32 lifeSpanTotal = particle->GemWarsLifeSpan->Total;
            F32 alpha = F32(lifeSpan) / lifeSpanTotal;
            particle->GemWarsMesh2D->Tint.a = alpha;
            particle->GemWarsLifeSpan->Remaining--;
        }
    }
}

void GemWarsExample::SetBounds()
{
    glm::vec2 bottomLeft = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, m_ViewportSize.y });
    glm::vec2 topRight   = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  0 });
    m_Bounds.BottomLeft = bottomLeft;
    m_Bounds.TopRight = topRight;
}

bool GemWarsExample::Collide(Entity& a, Entity& b)
{
    F32 minDistance = (a.GemWarsRigidBody2D->CollisionRadius + b.GemWarsRigidBody2D->CollisionRadius) * (a.GemWarsRigidBody2D->CollisionRadius + b.GemWarsRigidBody2D->CollisionRadius);
    return (glm::length2(glm::vec2(a.GemWarsTransform2D->Position) - glm::vec2(b.GemWarsTransform2D->Position)) < minDistance);
}

void GemWarsExample::OnEvent(Event& event)
{
    m_CameraController->OnEvent(event);
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>(BIND_FN(GemWarsExample::OnWindowResize));
}

bool GemWarsExample::OnWindowResize(WindowResizeEvent& event)
{
    SetBounds();
    return false;
}