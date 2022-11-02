#include "GemWarsExample.h"

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
    Entity* entity = &m_Manager.AddEntity("player");
    F32 playerRadius = 0.2f;
    entity->AddComponent<Component::GemWarsTransform2D>(glm::vec2{0.0f, 0.0f}, glm::vec2{playerRadius}, 0.0f);
    entity->AddComponent<Component::GemWarsRigidBody2D>(playerRadius, 2.0f);
    entity->AddComponent<Component::GemWarsMesh2D>(8, nullptr, glm::vec4{0.78f, 0.55f, 0.16f, 1.0f});
    entity->AddComponent<Component::GemWarsInput>();
    entity->AddComponent<Component::GemWarsSpecialAbility>(120);
    entity->AddComponent<Component::GemWarsScore>(0);
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
            glm::vec2 allowedXRegion = glm::vec2{
                m_Bounds.BottomLeft.x + enemyRadius, m_Bounds.TopRight.x - enemyRadius
            };
            glm::vec2 allowedYRegion = glm::vec2{
                m_Bounds.BottomLeft.y + enemyRadius, m_Bounds.TopRight.y - enemyRadius
            };
            glm::vec2 enemyPosition = glm::vec2(Random::Float(allowedXRegion.x, allowedXRegion.y),
                                                Random::Float(allowedYRegion.x, allowedYRegion.y));
            enemy.AddComponent<Component::GemWarsTransform2D>(enemyPosition, glm::vec2{enemyRadius}, 0.0f);
            enemy.AddComponent<Component::GemWarsRigidBody2D>(enemyRadius, Random::Float(2.0, 5.0f),
                                                              Random::Float(glm::radians(30.0f), glm::radians(90.0f)));
            while (Collide(enemy, *m_Player))
            {
                enemy.GetComponent<Component::GemWarsTransform2D>().Position = glm::vec3(
                    Random::Float(allowedXRegion.x, allowedXRegion.y),
                    Random::Float(allowedYRegion.x, allowedYRegion.y), 0.0f);
            }

            enemy.GetComponent<Component::GemWarsRigidBody2D>().Velocity = glm::normalize(Random::Float2(-1.0, 1.0));
            enemy.AddComponent<Component::GemWarsMesh2D>(Random::UInt(3, 8), nullptr,
                                                         glm::vec4(Random::Float3(0.2f, 0.6f), 1.0));
            enemy.AddComponent<Component::GemWarsScore>(
                enemy.GetComponent<Component::GemWarsMesh2D>().Shape.GetNumberOfVertices() * 10);
        }

        m_LastEnemySpawnTime = m_CurrentFrame;
    }
}

void GemWarsExample::SpawnParticles(Entity& entity)
{
    U32 numberOfParticles = entity.GetComponent<Component::GemWarsMesh2D>().Shape.GetNumberOfVertices();
    auto& transform2D = entity.GetComponent<Component::GemWarsTransform2D>();
    glm::vec2 particlesSize = transform2D.Scale / 2.0f;
    F32 particleSpeed = 2.0f;
    I32 particlesLifetime = 60;

    for (U32 i = 0; i < numberOfParticles; i++)
    {
        F32 phase = 2.0f * glm::pi<float>() * F32(i) / F32(numberOfParticles);
        F32 rotation = transform2D.Rotation;
        glm::vec3 particlePos = glm::vec3(glm::vec2(glm::sin(phase), glm::cos(phase)), 0.0f) * entity.GetComponent<
            Component::GemWarsRigidBody2D>().CollisionRadius;
        particlePos = {
            glm::cos(rotation) * particlePos.x - glm::sin(rotation) * particlePos.y,
            glm::sin(rotation) * particlePos.x + glm::cos(rotation) * particlePos.y,
            0.0f
        };
        particlePos += transform2D.Position;
        glm::vec2 particleVelocity = glm::normalize(particlePos - transform2D.Position);

        Entity& particle = m_Manager.AddEntity("particle");
        particle.AddComponent<Component::GemWarsTransform2D>(particlePos, particlesSize, 0.0f);
        auto& rb = particle.AddComponent<Component::GemWarsRigidBody2D>(particlesSize.x, particleSpeed);
        rb.Velocity = particleVelocity;
        particle.AddComponent<Component::GemWarsMesh2D>(numberOfParticles, nullptr,
                                                        entity.GetComponent<Component::GemWarsMesh2D>().Tint);
        particle.AddComponent<Component::GemWarsLifeSpan>(particlesLifetime);
    }
}

void GemWarsExample::SpawnBullet(Entity& entity, const glm::vec2& target)
{
    F32 bulletSpeed = 15.0f;
    F32 bulletRadius = 0.02f;
    glm::vec2 bulletVelocity = target - glm::vec2(entity.GetComponent<Component::GemWarsTransform2D>().Position);
    bulletVelocity += Random::Float2(-0.3f, 0.3f);
    bulletVelocity = glm::normalize(bulletVelocity);
    glm::vec2 bulletPosition = glm::vec2(entity.GetComponent<Component::GemWarsTransform2D>().Position) + entity.
        GetComponent<Component::GemWarsRigidBody2D>().CollisionRadius * bulletVelocity;
    Entity& bullet = m_Manager.AddEntity("bullet");
    bullet.AddComponent<Component::GemWarsTransform2D>(bulletPosition, glm::vec2{bulletRadius}, 0.0f);
    auto& rb = bullet.AddComponent<Component::GemWarsRigidBody2D>(bulletRadius, bulletSpeed);
    rb.Velocity = bulletVelocity;
    bullet.AddComponent<Component::GemWarsMesh2D>(4, nullptr, glm::vec4(0.6f, 0.9f, 0.2f, 1.0f));
}

void GemWarsExample::sSpecialAbility()
{
    static I32 cloud = 150;
    if (m_Player->GetComponent<Component::GemWarsInput>().SpecialAbility)
    {
        if (m_Player->GetComponent<Component::GemWarsSpecialAbility>().RemainingCoolDown == 0)
        {
            for (I32 i = 0; i < cloud; i++)
            {
                F32 phase = F32(i) / F32(cloud) * 2 * glm::pi<F32>();
                glm::vec2 position = {
                    glm::sin(phase) * m_Player->GetComponent<Component::GemWarsRigidBody2D>().CollisionRadius,
                    glm::cos(phase) * m_Player->GetComponent<Component::GemWarsRigidBody2D>().CollisionRadius
                };
                position += glm::vec2(m_Player->GetComponent<Component::GemWarsTransform2D>().Position);
                SpawnBullet(*m_Player, position);
            }
            m_Player->GetComponent<Component::GemWarsSpecialAbility>().RemainingCoolDown = m_Player->GetComponent<
                Component::GemWarsSpecialAbility>().CoolDown;
        }
    }
    if (m_Player->GetComponent<Component::GemWarsSpecialAbility>().RemainingCoolDown > 0) m_Player->GetComponent<
        Component::GemWarsSpecialAbility>().RemainingCoolDown--;
}

void GemWarsExample::sAddScore(Entity& entity)
{
    m_Player->GetComponent<Component::GemWarsScore>().TotalScore += entity.GetComponent<Component::GemWarsScore>().TotalScore;
    entity.GetComponent<Component::GemWarsScore>().TotalScore = 0;
}

void GemWarsExample::sMovement(F32 dt)
{
    // Update player.
    glm::vec2 velocity = glm::vec2(0.0f);
    if (m_Player->GetComponent<Component::GemWarsInput>().Left) velocity.x -= 1.0f;
    if (m_Player->GetComponent<Component::GemWarsInput>().Right) velocity.x += 1.0f;
    if (m_Player->GetComponent<Component::GemWarsInput>().Up) velocity.y += 1.0f;
    if (m_Player->GetComponent<Component::GemWarsInput>().Down) velocity.y -= 1.0f;
    if (velocity != glm::vec2(0.0f)) velocity = glm::normalize(velocity);
    m_Player->GetComponent<Component::GemWarsRigidBody2D>().Velocity = velocity;

    if (m_Player->GetComponent<Component::GemWarsInput>().Shoot) SpawnBullet(
        *m_Player, m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition()));
    // Update all.
    for (auto& entity : m_Manager.GetEntities())
    {
        entity->GetComponent<Component::GemWarsTransform2D>().Position += glm::vec3(
            entity->GetComponent<Component::GemWarsRigidBody2D>().Velocity * entity->GetComponent<
                Component::GemWarsRigidBody2D>().Speed * dt, 0.0f);
        entity->GetComponent<Component::GemWarsTransform2D>().Rotation += entity->GetComponent<
            Component::GemWarsRigidBody2D>().RotationSpeed * dt;
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
    F32 playerRadius = m_Player->GetComponent<Component::GemWarsRigidBody2D>().CollisionRadius;
    glm::vec3& playerPosition = m_Player->GetComponent<Component::GemWarsTransform2D>().Position;
    if (playerPosition.x + playerRadius > m_Bounds.TopRight.x) playerPosition.x = m_Bounds.TopRight.x - playerRadius;
    if (playerPosition.y + playerRadius > m_Bounds.TopRight.y) playerPosition.y = m_Bounds.TopRight.y - playerRadius;
    if (playerPosition.x - playerRadius < m_Bounds.BottomLeft.x) playerPosition.x = m_Bounds.BottomLeft.x +
        playerRadius;
    if (playerPosition.y - playerRadius < m_Bounds.BottomLeft.y) playerPosition.y = m_Bounds.BottomLeft.y +
        playerRadius;

    // Check for enemy-walls collision.
    for (auto& entity : m_Manager.GetEntities("enemy"))
    {
        if (entity->GetComponent<Component::GemWarsTransform2D>().Position.x + entity->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius > m_Bounds.TopRight.x ||
            entity->GetComponent<Component::GemWarsTransform2D>().Position.x - entity->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius < m_Bounds.BottomLeft.x)
        {
            entity->GetComponent<Component::GemWarsRigidBody2D>().Velocity.x *= -1.0f;
        }
        if (entity->GetComponent<Component::GemWarsTransform2D>().Position.y + entity->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius > m_Bounds.TopRight.y ||
            entity->GetComponent<Component::GemWarsTransform2D>().Position.y - entity->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius < m_Bounds.BottomLeft.y)
        {
            entity->GetComponent<Component::GemWarsRigidBody2D>().Velocity.y *= -1.0f;
        }
    }

    // Check bullet-enemy collision.
    for (auto& bullet : m_Manager.GetEntities("bullet"))
    {
        for (auto& enemy : m_Manager.GetEntities("enemy"))
        {
            if (Collide(*bullet, *enemy))
            {
                bullet->Destroy();
                enemy->Destroy();
                SpawnParticles(*enemy);
                sAddScore(*enemy);
            }
        }
    }

    // Check bullet-wall collision.
    for (auto& bullet : m_Manager.GetEntities("bullet"))
    {
        if (bullet->GetComponent<Component::GemWarsTransform2D>().Position.x - bullet->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius > m_Bounds.TopRight.x ||
            bullet->GetComponent<Component::GemWarsTransform2D>().Position.x + bullet->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius < m_Bounds.BottomLeft.x)
        {
            bullet->Destroy();
        }
        if (bullet->GetComponent<Component::GemWarsTransform2D>().Position.y - bullet->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius > m_Bounds.TopRight.y ||
            bullet->GetComponent<Component::GemWarsTransform2D>().Position.y + bullet->GetComponent<
                Component::GemWarsRigidBody2D>().CollisionRadius < m_Bounds.BottomLeft.y)
        {
            bullet->Destroy();
        }
    }
}

void GemWarsExample::sUserInput()
{
    m_Player->GetComponent<Component::GemWarsInput>().Left = Input::GetKey(Key::A);
    m_Player->GetComponent<Component::GemWarsInput>().Right = Input::GetKey(Key::D);
    m_Player->GetComponent<Component::GemWarsInput>().Up = Input::GetKey(Key::W);
    m_Player->GetComponent<Component::GemWarsInput>().Down = Input::GetKey(Key::S);
    m_Player->GetComponent<Component::GemWarsInput>().Shoot = Input::GetMouseButton(Mouse::Button0);
    m_Player->GetComponent<Component::GemWarsInput>().SpecialAbility = Input::GetMouseButton(Mouse::Button1);

    if (Input::GetKeyDown(Key::P)) SetPaused(m_IsRunning);
}

void GemWarsExample::sRender()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera().get());

    Component::Transform2D transform;
    transform.Scale = {40.0f, 40.0f};
    Component::SpriteRenderer sp;
    sp.Texture = m_Background.get();
    sp.Tiling = {20.0f, 20.0f};
    sp.OrderInLayer = -1;
    Renderer2D::DrawQuad(transform, sp);

    for (auto& entity : m_Manager.GetEntities())
    {
        auto& cTf = entity->GetComponent<Component::GemWarsTransform2D>();
        if (!entity->HasComponent<Component::GemWarsMesh2D>()) continue;
        transform.Position = cTf.Position;
        transform.Scale = cTf.Scale;
        transform.Rotation = cTf.Rotation;
        Component::PolygonRenderer pr;
        auto& mesh = entity->GetComponent<Component::GemWarsMesh2D>();
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
                                              m_Player->GetComponent<Component::GemWarsScore>().TotalScore));
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
    for (auto& particle : m_Manager.GetEntities("particle"))
    {
        I32 lifeSpan = particle->GetComponent<Component::GemWarsLifeSpan>().Remaining;
        if (lifeSpan <= 0) particle->Destroy();
        else
        {
            I32 lifeSpanTotal = particle->GetComponent<Component::GemWarsLifeSpan>().Total;
            F32 alpha = F32(lifeSpan) / F32(lifeSpanTotal);
            particle->GetComponent<Component::GemWarsMesh2D>().Tint.a = alpha;
            particle->GetComponent<Component::GemWarsLifeSpan>().Remaining--;
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

bool GemWarsExample::Collide(Entity& a, Entity& b)
{
    auto& rbA = a.GetComponent<Component::GemWarsRigidBody2D>();
    auto& rbB = b.GetComponent<Component::GemWarsRigidBody2D>();
    auto& tfA = a.GetComponent<Component::GemWarsTransform2D>();
    auto& tfB = b.GetComponent<Component::GemWarsTransform2D>();
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
