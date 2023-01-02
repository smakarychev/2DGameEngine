#include "GemWarsExample.h"

#include "Engine/ECS/View.h"

#include <ranges>

void GemWarsExample::OnAttach()
{
    m_Tileset = Texture::LoadTextureFromFile("assets/textures/cavesofgallet_tiles.png");
    m_Background = m_Tileset->GetSubTexture({8.0f, 8.0f}, {3, 11});
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = {Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight()};
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);

    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);
    camera->SetZoom(3.0f);

    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));

    SetBounds();

    SpawnPlayer();
    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");
    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth();
    spec.Height = camera->GetViewportHeight();
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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = {viewportSize.x, viewportSize.y};
    U64 textureID = m_FrameBuffer->GetColorBufferId(0);
    ImGui::Image(reinterpret_cast<void*>(textureID), viewportSize, ImVec2{0, 1}, ImVec2{1, 0});
    ImGui::PopStyleVar(1);
    ImGui::End();
}


void GemWarsExample::SpawnPlayer()
{
    Entity entity = m_Registry.CreateEntity("player");
    F32 playerRadius = 0.2f;
    m_Registry.Add<Component::GemWarsTransform2D>(entity, glm::vec2{0.0f, 0.0f}, glm::vec2{playerRadius}, 0.0f);
    m_Registry.Add<Component::GemWarsRigidBody2D>(entity, playerRadius, 2.0f);
    m_Registry.Add<Component::GemWarsMesh2D>(entity, 8, nullptr, glm::vec4{0.78f, 0.55f, 0.16f, 1.0f});
    m_Registry.Add<Component::GemWarsInput>(entity);
    m_Registry.Add<Component::GemWarsSpecialAbility>(entity, 120);
    m_Registry.Add<Component::GemWarsScore>(entity, 0);
    m_Registry.Add<Component::GemWarsPlayerTag>(entity);
    m_Player = entity;
    for (auto e : View<Component::GemWarsEnemyTag>(m_Registry))
    {
        m_Registry.DeleteEntity(e);
    }
    for (auto e : View<Component::GemWarsParticleTag>(m_Registry))
    {
        m_Registry.DeleteEntity(e);
    }
}

void GemWarsExample::sEnemySpawner()
{
    I32 spawnThreshold = 2 * 60;
    if (m_CurrentFrame - m_LastEnemySpawnTime > spawnThreshold)
    {
        U32 enemiesToSpawn = Random::UInt32(1, 3);
        for (U32 i = 0; i < enemiesToSpawn; i++)
        {
            Entity enemy = m_Registry.CreateEntity("enemy");
            F32 enemyRadius = Random::Float(0.15f, 0.45f);
            glm::vec2 allowedXRegion = glm::vec2{
                m_Bounds.BottomLeft.x + enemyRadius, m_Bounds.TopRight.x - enemyRadius
            };
            glm::vec2 allowedYRegion = glm::vec2{
                m_Bounds.BottomLeft.y + enemyRadius, m_Bounds.TopRight.y - enemyRadius
            };
            glm::vec2 enemyPosition = glm::vec2(Random::Float(allowedXRegion.x, allowedXRegion.y),
                                                Random::Float(allowedYRegion.x, allowedYRegion.y));
            m_Registry.Add<Component::GemWarsTransform2D>(enemy, enemyPosition, glm::vec2{enemyRadius}, 0.0f);
            m_Registry.Add<Component::GemWarsRigidBody2D>(enemy, enemyRadius, Random::Float(2.0, 5.0f),
                                                              Random::Float(glm::radians(30.0f), glm::radians(90.0f)));
            while (Collide(enemy, m_Player))
            {
                m_Registry.Get<Component::GemWarsTransform2D>(enemy).Position = glm::vec3(
                    Random::Float(allowedXRegion.x, allowedXRegion.y),
                    Random::Float(allowedYRegion.x, allowedYRegion.y), 0.0f);
            }

            m_Registry.Get<Component::GemWarsRigidBody2D>(enemy).Velocity = glm::normalize(Random::Float2(-1.0, 1.0));
            m_Registry.Add<Component::GemWarsMesh2D>(enemy, Random::UInt32(3, 8), nullptr,
                                                         glm::vec4(Random::Float3(0.2f, 0.6f), 1.0));
            m_Registry.Add<Component::GemWarsScore>(enemy, 
                m_Registry.Get<Component::GemWarsMesh2D>(enemy).Shape.GetNumberOfVertices() * 10);
            m_Registry.Add<Component::GemWarsEnemyTag>(enemy);
        }

        m_LastEnemySpawnTime = m_CurrentFrame;
    }
}

void GemWarsExample::SpawnParticles(Entity entity)
{
    U32 numberOfParticles = m_Registry.Get<Component::GemWarsMesh2D>(entity).Shape.GetNumberOfVertices();
    F32 particleSpeed = 2.0f;
    I32 particlesLifetime = 60;

    auto& transform2D = m_Registry.Get<Component::GemWarsTransform2D>(entity);
    for (U32 i = 0; i < numberOfParticles; i++)
    {
        glm::vec2 particlesSize = transform2D.Scale / 2.0f;
        F32 phase = 2.0f * glm::pi<float>() * F32(i) / F32(numberOfParticles);
        F32 rotation = transform2D.Rotation;
        glm::vec3 particlePos = glm::vec3(glm::vec2(glm::sin(phase), glm::cos(phase)), 0.0f) * m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius;
        particlePos = {
            glm::cos(rotation) * particlePos.x - glm::sin(rotation) * particlePos.y,
            glm::sin(rotation) * particlePos.x + glm::cos(rotation) * particlePos.y,
            0.0f
        };
        particlePos += transform2D.Position;
        glm::vec2 particleVelocity = glm::normalize(particlePos - transform2D.Position);

        Entity particle = m_Registry.CreateEntity("particle");
        m_Registry.Add<Component::GemWarsTransform2D>(particle, particlePos, particlesSize, 0.0f);
        auto& rb = m_Registry.Add<Component::GemWarsRigidBody2D>(particle, particlesSize.x, particleSpeed);
        rb.Velocity = particleVelocity;
        auto& gwm = m_Registry.Add<Component::GemWarsMesh2D>(particle, numberOfParticles, nullptr,
                                                        glm::vec4{1.0f});
        gwm.Tint = m_Registry.Get<Component::GemWarsMesh2D>(entity).Tint;
        m_Registry.Add<Component::GemWarsLifeSpan>(particle, particlesLifetime);
        m_Registry.Add<Component::GemWarsParticleTag>(particle);
    }
}

void GemWarsExample::SpawnBullet(Entity entity, const glm::vec2& target)
{
    F32 bulletSpeed = 15.0f;
    F32 bulletRadius = 0.02f;
    glm::vec2 bulletVelocity = target - glm::vec2(m_Registry.Get<Component::GemWarsTransform2D>(entity).Position);
    bulletVelocity += Random::Float2(-0.3f, 0.3f);
    bulletVelocity = glm::normalize(bulletVelocity);
    glm::vec2 bulletPosition = glm::vec2(m_Registry.Get<Component::GemWarsTransform2D>(entity).Position) +
        m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius * bulletVelocity;
    Entity bullet = m_Registry.CreateEntity("bullet");
    m_Registry.Add<Component::GemWarsTransform2D>(bullet, bulletPosition, glm::vec2{bulletRadius}, 0.0f);
    auto& rb = m_Registry.Add<Component::GemWarsRigidBody2D>(bullet, bulletRadius, bulletSpeed);
    rb.Velocity = bulletVelocity;
    m_Registry.Add<Component::GemWarsMesh2D>(bullet, 4, nullptr, glm::vec4(0.6f, 0.9f, 0.2f, 1.0f));
    m_Registry.Add<Component::GemWarsBulletTag>(bullet);
}

void GemWarsExample::sSpecialAbility()
{
    static I32 cloud = 150;
    if (m_Registry.Get<Component::GemWarsInput>(m_Player).SpecialAbility)
    {
        if (m_Registry.Get<Component::GemWarsSpecialAbility>(m_Player).RemainingCoolDown == 0)
        {
            for (I32 i = 0; i < cloud; i++)
            {
                F32 phase = F32(i) / F32(cloud) * 2 * glm::pi<F32>();
                glm::vec2 position = {
                    glm::sin(phase) * m_Registry.Get<Component::GemWarsRigidBody2D>(m_Player).CollisionRadius,
                    glm::cos(phase) * m_Registry.Get<Component::GemWarsRigidBody2D>(m_Player).CollisionRadius
                };
                position += glm::vec2(m_Registry.Get<Component::GemWarsTransform2D>(m_Player).Position);
                SpawnBullet(m_Player, position);
            }
            m_Registry.Get<Component::GemWarsSpecialAbility>(m_Player).RemainingCoolDown = m_Registry.Get<Component::GemWarsSpecialAbility>(m_Player).CoolDown;
        }
    }
    if (m_Registry.Get<Component::GemWarsSpecialAbility>(m_Player).RemainingCoolDown > 0) m_Registry.Get<Component::GemWarsSpecialAbility>(m_Player).RemainingCoolDown--;
}

void GemWarsExample::sAddScore(Entity entity)
{
    m_Registry.Get<Component::GemWarsScore>(m_Player).TotalScore +=  m_Registry.Get<Component::GemWarsScore>(entity).TotalScore;
    m_Registry.Get<Component::GemWarsScore>(entity).TotalScore = 0;
}

void GemWarsExample::sMovement(F32 dt)
{
    // Update player.
    glm::vec2 velocity = glm::vec2(0.0f);
    if (m_Registry.Get<Component::GemWarsInput>(m_Player).Left) velocity.x -= 1.0f;
    if (m_Registry.Get<Component::GemWarsInput>(m_Player).Right) velocity.x += 1.0f;
    if (m_Registry.Get<Component::GemWarsInput>(m_Player).Up) velocity.y += 1.0f;
    if (m_Registry.Get<Component::GemWarsInput>(m_Player).Down) velocity.y -= 1.0f;
    if (velocity != glm::vec2(0.0f)) velocity = glm::normalize(velocity);
    m_Registry.Get<Component::GemWarsRigidBody2D>(m_Player).Velocity = velocity;

    if (m_Registry.Get<Component::GemWarsInput>(m_Player).Shoot) SpawnBullet(
        m_Player, m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition()));
    // Update all.
    for (auto entity : View<>(m_Registry))
    {
        m_Registry.Get<Component::GemWarsTransform2D>(entity).Position += glm::vec3(
            m_Registry.Get<Component::GemWarsRigidBody2D>(entity).Velocity * m_Registry.Get<Component::GemWarsRigidBody2D>(entity).Speed * dt, 0.0f);
        m_Registry.Get<Component::GemWarsTransform2D>(entity).Rotation += m_Registry.Get<Component::GemWarsRigidBody2D>(entity).RotationSpeed * dt;
    }
}

void GemWarsExample::sCollision()
{
    // Check for player-enemy collision.
    for (auto entity : View<Component::GemWarsEnemyTag>(m_Registry))
    {
        if (Collide(m_Player, entity))
        {
            m_Registry.DeleteEntity(m_Player);
            SpawnPlayer();
            SetPaused(m_IsRunning);
            break;
        }
    }
    for (auto entity : View<Component::GemWarsParticleTag>(m_Registry))
    {
        if (Collide(m_Player, entity))
        {
            m_Registry.DeleteEntity(m_Player);
            SpawnPlayer();
            SetPaused(m_IsRunning);
            break;
        }
    }

    // Check for player-walls collision.
    F32 playerRadius = m_Registry.Get<Component::GemWarsRigidBody2D>(m_Player).CollisionRadius;
    glm::vec3& playerPosition = m_Registry.Get<Component::GemWarsTransform2D>(m_Player).Position;
    if (playerPosition.x + playerRadius > m_Bounds.TopRight.x) playerPosition.x = m_Bounds.TopRight.x - playerRadius;
    if (playerPosition.y + playerRadius > m_Bounds.TopRight.y) playerPosition.y = m_Bounds.TopRight.y - playerRadius;
    if (playerPosition.x - playerRadius < m_Bounds.BottomLeft.x) playerPosition.x = m_Bounds.BottomLeft.x +
        playerRadius;
    if (playerPosition.y - playerRadius < m_Bounds.BottomLeft.y) playerPosition.y = m_Bounds.BottomLeft.y +
        playerRadius;

    // Check for enemy-walls collision.
    for (auto entity : View<Component::GemWarsEnemyTag>(m_Registry))
    {
        if (m_Registry.Get<Component::GemWarsTransform2D>(entity).Position.x + m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius > m_Bounds.TopRight.x ||
            m_Registry.Get<Component::GemWarsTransform2D>(entity).Position.x - m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius < m_Bounds.BottomLeft.x)
        {
            m_Registry.Get<Component::GemWarsRigidBody2D>(entity).Velocity.x *= -1.0f;
        }
        if (m_Registry.Get<Component::GemWarsTransform2D>(entity).Position.y + m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius > m_Bounds.TopRight.y ||
            m_Registry.Get<Component::GemWarsTransform2D>(entity).Position.y - m_Registry.Get<Component::GemWarsRigidBody2D>(entity).CollisionRadius < m_Bounds.BottomLeft.y)
        {
            m_Registry.Get<Component::GemWarsRigidBody2D>(entity).Velocity.y *= -1.0f;
        }
    }

    // Check bullet-enemy collision.
    for (auto bullet : View<Component::GemWarsBulletTag>(m_Registry))
    {
        for (auto enemy : View<Component::GemWarsEnemyTag>(m_Registry))
        {
            if (Collide(bullet, enemy))
            {
                SpawnParticles(enemy);
                sAddScore(enemy);
                m_Registry.DeleteEntity(bullet);
                m_Registry.DeleteEntity(enemy);
                break;
            }
        }
    }
    // Check bullet-wall collision.
    for (auto bullet : View<Component::GemWarsBulletTag>(m_Registry))
    {
        if (m_Registry.Get<Component::GemWarsTransform2D>(bullet).Position.x - m_Registry.Get<Component::GemWarsRigidBody2D>(bullet).CollisionRadius > m_Bounds.TopRight.x ||
            m_Registry.Get<Component::GemWarsTransform2D>(bullet).Position.x + m_Registry.Get<Component::GemWarsRigidBody2D>(bullet).CollisionRadius < m_Bounds.BottomLeft.x)
        {
            m_Registry.DeleteEntity(bullet);
            continue;
        }
        if (m_Registry.Get<Component::GemWarsTransform2D>(bullet).Position.y - m_Registry.Get<Component::GemWarsRigidBody2D>(bullet).CollisionRadius > m_Bounds.TopRight.y ||
            m_Registry.Get<Component::GemWarsTransform2D>(bullet).Position.y + m_Registry.Get<Component::GemWarsRigidBody2D>(bullet).CollisionRadius < m_Bounds.BottomLeft.y)
        {
            m_Registry.DeleteEntity(bullet);
        }
    }
}

void GemWarsExample::sUserInput()
{
    m_Registry.Get<Component::GemWarsInput>(m_Player).Left = Input::GetKey(Key::A);
    m_Registry.Get<Component::GemWarsInput>(m_Player).Right = Input::GetKey(Key::D);
    m_Registry.Get<Component::GemWarsInput>(m_Player).Up = Input::GetKey(Key::W);
    m_Registry.Get<Component::GemWarsInput>(m_Player).Down = Input::GetKey(Key::S);
    m_Registry.Get<Component::GemWarsInput>(m_Player).Shoot = Input::GetMouseButton(Mouse::Button0) || Input::GetKeyDown(Key::E);
    m_Registry.Get<Component::GemWarsInput>(m_Player).SpecialAbility = Input::GetMouseButton(Mouse::Button1);

    if (Input::GetKeyDown(Key::Space))
    {
        bool debugBreak = true;
    }

    if (Input::GetKeyDown(Key::P)) SetPaused(m_IsRunning);
}

void GemWarsExample::sRender()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera().get());

    Component::LocalToWorldTransform2D transform;
    transform.Scale = {40.0f, 40.0f};
    Component::SpriteRenderer sp;
    sp.Texture = m_Background.get();
    sp.Tiling = {20.0f, 20.0f};
    sp.OrderInLayer = -1;
    Renderer2D::DrawQuad(transform, sp);

    for (const auto e: View<Component::GemWarsTransform2D, Component::GemWarsMesh2D>(m_Registry))
    {
        auto& cTf = m_Registry.Get<Component::GemWarsTransform2D>(e);
        transform.Position = cTf.Position;
        transform.Scale = cTf.Scale;
        transform.Rotation = cTf.Rotation;
        Component::PolygonRenderer pr;
        auto& mesh = m_Registry.Get<Component::GemWarsMesh2D>(e);
        pr.Polygon = &mesh.Shape;
        pr.Tint = mesh.Tint;
        pr.OrderInLayer = 1;
        Renderer2D::DrawPolygon(transform, pr);
    }
 
    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.Tint = {0.6f, 0.9f, 0.7f, 1.0f};
    fr.FontSize = 36.0f;
    fr.FontRect = {.Min = {10.0f, 10.0f}, .Max = {1600.0f, 10.0f}};
    Renderer2D::DrawFontFixed(fr, std::format("score: {:d}",
                                              m_Registry.Get<Component::GemWarsScore>(m_Player).TotalScore));
    fr.FontSize = 14.0f;
    fr.FontRect = {
        .Min = {(F32)m_FrameBuffer->GetSpec().Width - 90.0f, 10.0f}, .Max = {(F32)m_FrameBuffer->GetSpec().Width, 10.0f}
    };
    Renderer2D::DrawFontFixed(fr, "GemWars example");

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
    for (auto particle : View<Component::GemWarsParticleTag>(m_Registry))
    {
        I32 lifeSpan = m_Registry.Get<Component::GemWarsLifeSpan>(particle).Remaining;
        if (lifeSpan <= 0)
        {
            m_Registry.DeleteEntity(particle);
        }
        else
        {
            I32 lifeSpanTotal = m_Registry.Get<Component::GemWarsLifeSpan>(particle).Total;
            F32 alpha = F32(lifeSpan) / F32(lifeSpanTotal);
            m_Registry.Get<Component::GemWarsMesh2D>(particle).Tint.a = alpha;
            m_Registry.Get<Component::GemWarsLifeSpan>(particle).Remaining--;
        }
    }
}

void GemWarsExample::SetBounds()
{
    glm::vec2 bottomLeft = m_CameraController->GetCamera()->ScreenToWorldPoint({0, 0});
    glm::vec2 topRight = m_CameraController->GetCamera()->ScreenToWorldPoint({m_ViewportSize.x, m_ViewportSize.y});
    m_Bounds.BottomLeft = bottomLeft;
    m_Bounds.TopRight = topRight;
}

bool GemWarsExample::Collide(Entity a, Entity b)
{
    auto& rbA = m_Registry.Get<Component::GemWarsRigidBody2D>(a);
    auto& rbB = m_Registry.Get<Component::GemWarsRigidBody2D>(b);
    auto& tfA = m_Registry.Get<Component::GemWarsTransform2D>(a);
    auto& tfB = m_Registry.Get<Component::GemWarsTransform2D>(b);
    F32 minDistance = (rbA.CollisionRadius + rbB.CollisionRadius) * (rbA.CollisionRadius + rbB.CollisionRadius);
    return (glm::length2(glm::vec2(tfA.Position) - glm::vec2(tfB.Position)) < minDistance);
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
