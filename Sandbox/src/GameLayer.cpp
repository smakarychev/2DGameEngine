#include "GameLayer.h"

#include "GLFW/glfw3.h"

void GameLayer::OnAttach()
{
    m_Tileset = Texture::LoadTextureFromFile("assets/textures/cavesofgallet_tiles.png");
    m_Background = m_Tileset->GetSubTexture({ 8.0f, 8.0f }, { 3, 11 });
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    camera->SetViewport(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);

    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));

    SetBounds();

    SpawnPlayer();

    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Regular.ttf");
}

void GameLayer::OnUpdate()
{
    RenderCommand::ClearScreen();
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);


    sUserInput();
    if (m_IsRunning)
    {
        m_Manager.Update();
        sCollision();
        sMovement(dt);
        sEnemySpawner();
        sParticleUpdate();
        sSpecialAbility();
        // Let's hope player doesn't play for more than 35 trillion years.
        m_CurrentFrame++;
    }
    sRender();
}

void GameLayer::OnEvent(Event& event)
{
    m_CameraController->OnEvent(event);
}

void GameLayer::SpawnPlayer()
{
    Entity* entity = &m_Manager.AddEntity("player");
    F32 playerRadius = 0.2f;
    entity->Transform2D = CreateRef<Component::Tranform2D>(glm::vec2{ 0.0f, 0.0f }, glm::vec2{ playerRadius }, 0.0f);
    entity->RigidBody2D = CreateRef<Component::RigidBody2D>(playerRadius, 2.0f);
    entity->Mesh2D = CreateRef<Component::Mesh2D>(8, nullptr, glm::vec4{ 0.78f, 0.55f, 0.16f, 1.0f });
    entity->Input = CreateRef<Component::Input>();
    entity->SpecialAbility = CreateRef<Component::SpecialAbility>(120);
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

void GameLayer::sEnemySpawner()
{
    I32 spawnThreshold = 2 * 60;
    if (m_CurrentFrame - m_LastEnemySpawnTime > spawnThreshold)
    {
        U32 enemiesToSpawn = Random::UInt(1, 3);
        for (U32 i = 0; i < enemiesToSpawn; i++)
        {
            Entity& entity = m_Manager.AddEntity("enemy");
            F32 enemyRadius = Random::Float(0.15f, 0.45f);
            glm::vec2 allowedXRegion = glm::vec2{ m_Bounds.BottomLeft.x + enemyRadius, m_Bounds.TopRight.x - enemyRadius };
            glm::vec2 allowedYRegion = glm::vec2{ m_Bounds.BottomLeft.y + enemyRadius, m_Bounds.TopRight.y - enemyRadius };
            glm::vec2 enemyPosition = glm::vec2(Random::Float(allowedXRegion.x, allowedXRegion.y), Random::Float(allowedYRegion.x, allowedYRegion.y));
            entity.Transform2D = CreateRef<Component::Tranform2D>(enemyPosition, glm::vec2{ enemyRadius }, 0.0f);
            entity.RigidBody2D = CreateRef<Component::RigidBody2D>(enemyRadius, Random::Float(2.0, 5.0f), Random::Float(glm::radians(30.0f), glm::radians(90.0f)));
            while (Collide(entity, *m_Player))
            {
                entity.Transform2D->Position = glm::vec3(Random::Float(allowedXRegion.x, allowedXRegion.y), Random::Float(allowedYRegion.x, allowedYRegion.y), 0.0f);
            }

            entity.RigidBody2D->Velocity = glm::normalize(Random::Float2(-1.0, 1.0));
            entity.Mesh2D = CreateRef<Component::Mesh2D>(Random::UInt(3, 8), nullptr, glm::vec4(Random::Float3(0.2, 0.6), 1.0));
        }
       
        m_LastEnemySpawnTime = m_CurrentFrame;
    }
}

void GameLayer::SpawnParticles(Entity& entity)
{
    U32 numberOfParticles = entity.Mesh2D->Shape.GetNumberOfVertices();
    glm::vec2 particlesSize = entity.Transform2D->Scale / 2.0f;
    F32 particleSpeed = 2.0f;
    I32 particlesLifetime = 60;

    for (U32 i = 0; i < numberOfParticles; i++)
    {
        F32 phase = 2.0f * glm::pi<float>() * i / numberOfParticles;
        F32 rotation = entity.Transform2D->Rotation;
        glm::vec3 particlePos = glm::vec3(glm::vec2(glm::sin(phase), glm::cos(phase)), 0.0f) * entity.RigidBody2D->CollisionRadius;
        particlePos = {
            glm::cos(rotation) * particlePos.x - glm::sin(rotation) * particlePos.y,
            glm::sin(rotation) * particlePos.x + glm::cos(rotation) * particlePos.y,
            0.0f
        };
        particlePos += entity.Transform2D->Position;
        glm::vec2 particleVelocity = glm::normalize(particlePos - entity.Transform2D->Position);

        Entity& particle = m_Manager.AddEntity("particle");
        particle.Transform2D = CreateRef<Component::Tranform2D>(particlePos, particlesSize, 0.0f);
        particle.RigidBody2D = CreateRef<Component::RigidBody2D>(particlesSize.x, particleSpeed);
        particle.RigidBody2D->Velocity = particleVelocity;
        particle.Mesh2D = CreateRef<Component::Mesh2D>(numberOfParticles, nullptr, entity.Mesh2D->Tint);
        particle.LifeSpan = CreateRef<Component::LifeSpan>(particlesLifetime);
    }

}

void GameLayer::SpawnBullet(Entity& entity, const glm::vec2& target)
{
    F32 bulletSpeed = 15.0f;
    F32 bulletRadius = 0.02f;
    glm::vec2 bulletVelocity = target - glm::vec2(entity.Transform2D->Position);
    bulletVelocity += Random::Float2(-0.3, 0.3);
    bulletVelocity = glm::normalize(bulletVelocity);
    glm::vec2 bulletPosition = glm::vec2(entity.Transform2D->Position) + entity.RigidBody2D->CollisionRadius * bulletVelocity;
    Entity& bullet = m_Manager.AddEntity("bullet");
    bullet.Transform2D = CreateRef<Component::Tranform2D>(bulletPosition, glm::vec2{ bulletRadius }, 0.0f);
    bullet.RigidBody2D = CreateRef<Component::RigidBody2D>(bulletRadius, bulletSpeed);
    bullet.RigidBody2D->Velocity = bulletVelocity;
    bullet.Mesh2D = CreateRef<Component::Mesh2D>(4, nullptr, glm::vec4(0.6f, 0.9f, 0.2f, 1.0f));
}

void GameLayer::sSpecialAbility()
{
    static I32 cloud = 150;
    if (m_Player->Input->SpecialAbility)
    {
        if (m_Player->SpecialAbility->RemainingCoolDown == 0)
        {
            for (I32 i = 0; i < cloud; i++)
            {
                F32 phase = float(i) / cloud * 2 * glm::pi<F32>();
                glm::vec2 position = { glm::sin(phase) * m_Player->RigidBody2D->CollisionRadius, glm::cos(phase) * m_Player->RigidBody2D->CollisionRadius };
                position += glm::vec2(m_Player->Transform2D->Position);
                SpawnBullet(*m_Player, position);
            }
            m_Player->SpecialAbility->RemainingCoolDown = m_Player->SpecialAbility->CoolDown;
        }
    }
    if (m_Player->SpecialAbility->RemainingCoolDown > 0) m_Player->SpecialAbility->RemainingCoolDown--;

}

void GameLayer::sMovement(F32 dt)
{
    // Update player.
    glm::vec2 velocity = glm::vec2(0.0f);
    if (m_Player->Input->Left)  velocity.x -= 1.0f;
    if (m_Player->Input->Right) velocity.x += 1.0f;
    if (m_Player->Input->Up)    velocity.y += 1.0f;
    if (m_Player->Input->Down)  velocity.y -= 1.0f;
    if (velocity != glm::vec2(0.0f)) velocity = glm::normalize(velocity);
    m_Player->RigidBody2D->Velocity = velocity;

    if (m_Player->Input->Shoot) SpawnBullet(*m_Player, m_CameraController->GetCamera()->ScreenToWorldPoint(Input::MousePosition()));
    // Update all.
    for (auto& entity : m_Manager.GetEntities())
    {
        entity->Transform2D->Position += glm::vec3(entity->RigidBody2D->Velocity * entity->RigidBody2D->Speed * dt, 0.0f);
        entity->Transform2D->Rotation += entity->RigidBody2D->RotationSpeed * dt;
    }
       
}

void GameLayer::sCollision()
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
    F32 playerRadius = m_Player->RigidBody2D->CollisionRadius;
    glm::vec3& playerPosition = m_Player->Transform2D->Position;
    if (playerPosition.x + playerRadius > m_Bounds.TopRight.x) playerPosition.x = m_Bounds.TopRight.x - playerRadius;
    if (playerPosition.y + playerRadius > m_Bounds.TopRight.y) playerPosition.y = m_Bounds.TopRight.y - playerRadius;
    if (playerPosition.x - playerRadius < m_Bounds.BottomLeft.x) playerPosition.x = m_Bounds.BottomLeft.x + playerRadius;
    if (playerPosition.y - playerRadius < m_Bounds.BottomLeft.y) playerPosition.y = m_Bounds.BottomLeft.y + playerRadius;
    
    // Check for enemy-walls collision.
    for (auto& entity : m_Manager.GetEntities("enemy"))
    {
        if (entity->Transform2D->Position.x + entity->RigidBody2D->CollisionRadius > m_Bounds.TopRight.x ||
            entity->Transform2D->Position.x - entity->RigidBody2D->CollisionRadius < m_Bounds.BottomLeft.x)
        {
            entity->RigidBody2D->Velocity.x *= -1.0f;
        }
        if (entity->Transform2D->Position.y + entity->RigidBody2D->CollisionRadius > m_Bounds.TopRight.y ||
            entity->Transform2D->Position.y - entity->RigidBody2D->CollisionRadius < m_Bounds.BottomLeft.y)
        {
            entity->RigidBody2D->Velocity.y *= -1.0f;
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
            }
        }
    }

    // Check bullet-wall collision.
    for (auto& bullet : m_Manager.GetEntities("bullet"))
    {
        if (bullet->Transform2D->Position.x - bullet->RigidBody2D->CollisionRadius > m_Bounds.TopRight.x ||
            bullet->Transform2D->Position.x + bullet->RigidBody2D->CollisionRadius < m_Bounds.BottomLeft.x)
        {
            bullet->Destroy();
        }
        if (bullet->Transform2D->Position.y - bullet->RigidBody2D->CollisionRadius > m_Bounds.TopRight.y ||
            bullet->Transform2D->Position.y + bullet->RigidBody2D->CollisionRadius < m_Bounds.BottomLeft.y)
        {
            bullet->Destroy();
        }
    }
}

void GameLayer::sUserInput()
{
    m_Player->Input->Left = Input::GetKey(Key::A);
    m_Player->Input->Right = Input::GetKey(Key::D);
    m_Player->Input->Up = Input::GetKey(Key::W);
    m_Player->Input->Down = Input::GetKey(Key::S);
    m_Player->Input->Shoot = Input::GetMouseButton(Mouse::Button0);
    m_Player->Input->SpecialAbility = Input::GetMouseButton(Mouse::Button1);

    if (Input::GetKeyDown(Key::P)) SetPaused(m_IsRunning);
}

void GameLayer::sRender()
{
    Renderer2D::BeginScene(m_CameraController->GetCamera());

    /*Renderer2D::DrawQuad({ 0.0f, 0.0f, -10.0f }, { 20.0f, 20.0f }, *m_Background, { 1, 1 });

    for (auto& entity : m_Manager.GetEntities())
    {
        if (!entity->Mesh2D) continue;
        Renderer2D::DrawPolygon(entity->Mesh2D->Shape, entity->Transform2D->Position, entity->Transform2D->Scale, entity->Transform2D->Rotation, entity->Mesh2D->Tint);
    }*/

    F32 advance = 0.0f;
    for (int i = 65; i < 100; i++)
    {
        Renderer2D::DrawQuad(
            glm::vec3{ advance, 0.0f, 0.0f } + glm::vec3(m_Font->GetCharacters()[i].Bearing, 0.0f),
            m_Font->GetCharacters()[i].Size,
            m_Font->GetAtlas(),
            m_Font->GetCharacters()[i].UV, 
            glm::vec4(1.0f)
        );
        advance += m_Font->GetCharacters()[i].Advance;
    }
    

    Renderer2D::EndScene();
}

void GameLayer::sParticleUpdate()
{
    for (auto& particle : m_Manager.GetEntities("particle"))
    {
        I32 lifeSpan = particle->LifeSpan->Remaining;
        if (lifeSpan <= 0) particle->Destroy();
        else
        {
            I32 lifeSpanTotal = particle->LifeSpan->Total;
            F32 alpha = F32(lifeSpan) / lifeSpanTotal;
            particle->Mesh2D->Tint.a = alpha;
            particle->LifeSpan->Remaining--;
        }
    }
}

void GameLayer::SetBounds()
{
    glm::vec2 bottomLeft = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, Application::Get().GetWindow().GetHeight() });
    glm::vec2 topRight   = m_CameraController->GetCamera()->ScreenToWorldPoint({ Application::Get().GetWindow().GetWidth(),  0 });
    m_Bounds.BottomLeft = bottomLeft;
    m_Bounds.TopRight = topRight;
}

bool GameLayer::Collide(Entity& a, Entity& b)
{
    F32 minDistance = (a.RigidBody2D->CollisionRadius + b.RigidBody2D->CollisionRadius) * (a.RigidBody2D->CollisionRadius + b.RigidBody2D->CollisionRadius);
    return (glm::length2(glm::vec2(a.Transform2D->Position) - glm::vec2(b.Transform2D->Position)) < minDistance);
}