#include "MarioScene.h"

#include "MarioActions.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/SceneUtils.h"

void MarioScene::OnInit()
{
    // Create sorting layer for correct render order.
    // The structure is Background->Middleground->Default.
    m_SortingLayer.CreateLayer("Background");
    m_SortingLayer.CreateLayer("Middleground");
    m_SortingLayer.PlaceBefore(m_SortingLayer.GetLayer("Middleground"), m_SortingLayer.GetLayer("Default"));
    m_SortingLayer.PlaceBefore(m_SortingLayer.GetLayer("Background"), m_SortingLayer.GetLayer("Middleground"));
    
    // Set actions.
    RegisterAction(Key::A, CreateRef<MoveLeftAction>(m_Registry));
    RegisterAction(Key::D, CreateRef<MoveRightAction>(m_Registry));
    RegisterAction(Key::Space, CreateRef<JumpAction>(m_Registry));

    // Adjust gravity for better feel.
    m_RigidBodyWorld2D.SetGravity({0.0f, -25.0f});
    // Set custom contact listener.
    m_RigidBodyWorld2D.SetContactListener(&m_ContactListener);
    m_ContactListener.SetRegistry(&m_Registry);

    // Load assets.
    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");
    m_BrickTexture = Texture::LoadTextureFromFile("assets/textures/mario/bricks.png");
    m_MarioSprites = Texture::LoadTextureFromFile("assets/textures/mario/mario_sprites.png");
    Ref<SpriteAnimation> walkRight = CreateRef<SpriteAnimation>(
        m_MarioSprites.get(),
        glm::uvec2{235, 410}, glm::uvec2{30, 28}, 3, 10
    );
    Ref<SpriteAnimation> walkLeft = CreateRef<SpriteAnimation>(
        m_MarioSprites.get(),
        glm::uvec2{85, 410}, glm::uvec2{30, 28}, 3, 10
    );
    Ref<SpriteAnimation> jumpRight = CreateRef<SpriteAnimation>(
        m_MarioSprites.get(),
        glm::uvec2{295, 410}, glm::uvec2{30, 28}, 1, 0
    );
    Ref<SpriteAnimation> jumpLeft = CreateRef<SpriteAnimation>(
        m_MarioSprites.get(),
        glm::uvec2{25, 410}, glm::uvec2{30, 28}, 1, 0
    );
    Ref<SpriteAnimation> stand = CreateRef<SpriteAnimation>(
        m_MarioSprites.get(),
        glm::uvec2{205, 410}, glm::uvec2{30, 28}, 1, 0
    );
    m_Animations.push_back(walkRight);
    m_Animations.push_back(walkLeft);
    m_Animations.push_back(jumpRight);
    m_Animations.push_back(jumpLeft);
    m_Animations.push_back(stand);
    m_AnimationsMap["wRight"] = walkRight.get();
    m_AnimationsMap["wLeft"] = walkLeft.get();
    m_AnimationsMap["jRight"] = jumpRight.get();
    m_AnimationsMap["jLeft"] = jumpLeft.get();
    m_AnimationsMap["stand"] = stand.get();

    // Add entities.
    m_ActiveEntity.Id = m_Registry.GetEntityManager().GetNullEntityFlag();
    CreateLevel();
    AddPlayer();
}

void MarioScene::OnUpdate(F32 dt)
{
    // Call systems.
    SMove();
    SPhysics(dt);
    SState();
    SAnimation(dt);

    // Find active entity here, since at this point in time framebuffer is surely bound.
    FindActiveEntity();
}

void MarioScene::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(MarioScene::OnMousePressed));
}

void MarioScene::OnRender()
{
    const auto& sortingLayer = Renderer2D::GetSortingLayer();
    Renderer2D::SetSortingLayer(m_SortingLayer);
    SRender();
    Renderer2D::SetSortingLayer(sortingLayer);

    // TODO: toggle debug draw.
    RigidBodyWorldDrawer::Draw(m_RigidBodyWorld2D);
}

void MarioScene::OnImguiRender()
{
    ImGui::Begin("Inspector");
    if (m_ActiveEntity.Id != m_Registry.GetEntityManager().GetNullEntityFlag())
    {
        auto& transform = m_Registry.Get<Component::Transform2D>(m_ActiveEntity);
        F32 rotationDegrees = glm::degrees(std::acos(transform.Rotation[0]));
        if (transform.Rotation[1] < 0.0f) rotationDegrees *= -1.0f;
        ImGui::Begin("Transform2D");
        ImGui::DragFloat2("Position", &transform.Position[0], 0.05f, -10.0f, 10.0f);
        ImGui::DragFloat("Rotation", &rotationDegrees, 0.05f, -360.0f, 360.0f);
        ImGui::DragFloat2("Scale", &transform.Scale[0], 0.05f, -10.0f, 10.0f);
        const F32 rotationRadians = glm::radians(rotationDegrees);
        transform.Rotation = glm::vec2{glm::cos(rotationRadians), glm::sin(rotationRadians)};
        if (m_Registry.Has<Component::BoxCollider2D>(m_ActiveEntity))
        {
            auto& bx = m_Registry.Get<Component::BoxCollider2D>(m_ActiveEntity);
            ImGui::Begin("BoxCollider2D");
            ImGui::DragFloat2("Position", &bx.Offset[0], 0.05f, -10.0f, 10.0f);
            ImGui::DragFloat2("Scale", &bx.HalfSize[0], 0.05f, 0.0f, 10.0f);
            ImGui::End();
        }
        if (m_Registry.Has<Component::RigidBody2D>(m_ActiveEntity))
        {
            SceneUtils::SynchronizePhysics(m_Registry, m_ActiveEntity, m_RigidBodyWorld2D);
        }
        ImGui::End();
    }

    ImGui::End();
}

void MarioScene::PerformAction(Action& action)
{
    action.Execute();
}

void MarioScene::SPhysics(F32 dt)
{
    m_RigidBodyWorld2D.Update(dt);
    for (const auto& e : View<Component::RigidBody2D>(m_Registry))
    {
        SceneUtils::SynchronizeWithPhysics(m_Registry, e, m_RigidBodyWorld2D);
    }
}

void MarioScene::SRender()
{
    for (const auto& e : View<Component::SpriteRenderer>(m_Registry))
    {
        const U32 entityId = e.Id;
        auto& tf = m_Registry.Get<Component::Transform2D>(e);
        auto& sr = m_Registry.Get<Component::SpriteRenderer>(e);
        Renderer2D::DrawQuadEditor(entityId, tf, sr);
    }
}

void MarioScene::SMove()
{
    static constexpr auto MAX_HORIZONTAL_SPEED = 6.0f;
    for (const auto& e : View<Component::MarioInput>(m_Registry))
    {
        // Update player position.
        auto& input = m_Registry.Get<Component::MarioInput>(e);
        auto& state = m_Registry.Get<Component::MarioState>(e);
        auto& rb = m_Registry.Get<Component::RigidBody2D>(e);
        if (input.Jump && input.CanJump)
            rb.PhysicsBody->AddForce(glm::vec2{0.0f, 10.0f},
                                     Physics::ForceMode::Impulse);
        if (input.Left) rb.PhysicsBody->AddForce(glm::vec2{-90.0f, 0.0f});
        if (input.Right) rb.PhysicsBody->AddForce(glm::vec2{90.0f, 0.0f});
        glm::vec2 vel = rb.PhysicsBody->GetLinearVelocity();
        vel.x = Math::Clamp(vel.x, -MAX_HORIZONTAL_SPEED, MAX_HORIZONTAL_SPEED);
        // If no input while on the ground, rapidly decrease velocity.
        if (input.Jump == false && input.CanJump == false && vel.y > 0.0f)
        {
            vel.y *= 0.9f;
        }
        if (input.None)
        {
            if (input.CanJump)
            {
                vel *= 0.9f;
            }
            else
            {
                vel.x *= 0.95f;
            }
        }
        if (input.Left && state.IsMovingRight || input.Right && state.IsMovingLeft)
        {
            vel.x *= 0.9f;
        }

        rb.PhysicsBody->SetLinearVelocity({vel.x, vel.y});
        input.None = true;
    }
}

void MarioScene::SAnimation(F32 dt)
{
    for (const auto& e : View<Component::Animation, Component::SpriteRenderer>(m_Registry))
    {
        const auto& animation = m_Registry.Get<Component::Animation>(e);
        animation.SpriteAnimation->Update(dt);
        if (animation.SpriteAnimation->HasEnded())
        {
            m_Registry.Remove<Component::Animation>(e);
        }
        // Update entity's sprite.
        auto& sr = m_Registry.Get<Component::SpriteRenderer>(e);
        sr.UV = animation.SpriteAnimation->GetCurrentFrameUV();
    }

    for (const auto& e : View<Component::Animation, Component::MarioState>(m_Registry))
    {
        auto& animation = m_Registry.Get<Component::Animation>(e);
        const auto& state = m_Registry.Get<Component::MarioState>(e);
        if (state.IsMovingLeft)
        {
            if (state.IsInMidAir && animation.SpriteAnimation != m_AnimationsMap["jLeft"])
            {
                animation.SpriteAnimation = m_AnimationsMap["jLeft"];
            }
            else if (!state.IsInMidAir && animation.SpriteAnimation != m_AnimationsMap["wLeft"])
            {
                animation.SpriteAnimation = m_AnimationsMap["wLeft"];
            }
        }
        else if (state.IsMovingRight)
        {
            if (state.IsInMidAir && animation.SpriteAnimation != m_AnimationsMap["jRight"])
            {
                animation.SpriteAnimation = m_AnimationsMap["jRight"];
            }
            else if (!state.IsInMidAir && animation.SpriteAnimation != m_AnimationsMap["wRight"])
            {
                animation.SpriteAnimation = m_AnimationsMap["wRight"];
            }
        }
        else
        {
            animation.SpriteAnimation = m_AnimationsMap["stand"];
        }
    }
}

void MarioScene::SState()
{
    for (const auto& e : View<Component::RigidBody2D, Component::MarioState>(m_Registry))
    {
        // Update state based on rigid body characteristics.
        const auto& rb = m_Registry.Get<Component::RigidBody2D>(e);
        auto& state = m_Registry.Get<Component::MarioState>(e);
        const auto& vel = rb.PhysicsBody->GetLinearVelocity();
        state.IsMovingLeft = vel.x < -0.01f;
        state.IsMovingRight = vel.x > 0.01f;
        state.IsInFreeFall = vel.y < 0.0f;
        state.IsInMidAir = state.IsInFreeFall || vel.y > 0.0f;
    }
}

void MarioScene::AddPlayer()
{
    Entity player = m_Registry.CreateEntity("player");
    auto& tf = m_Registry.Add<Component::Transform2D>(player);
    auto& rb = m_Registry.Add<Component::RigidBody2D>(player);
    auto& col = m_Registry.Add<Component::BoxCollider2D>(player);
    m_Registry.Add<Component::MarioInput>(player);
    m_Registry.Add<Component::MarioState>(player);

    tf.Scale = glm::vec2{1.0f, 1.75f};
    tf.Position = glm::vec2{0.0f, 0.0f};
    rb.Type = Physics::RigidBodyType2D::Dynamic;
    rb.Flags = Physics::RigidBodyDef2D::BodyFlags::RestrictRotation;

    // Create foot sensor.
    col.Next = New<Component::BoxCollider2D>();
    Component::BoxCollider2D& footSensor = *col.Next;
    footSensor.Offset.y = -col.HalfSize.y * tf.Scale.y;
    footSensor.HalfSize.x = col.HalfSize.x * 0.85f;
    footSensor.HalfSize.y = col.HalfSize.y * 0.15f;
    footSensor.IsSensor = true;
    SceneUtils::SynchronizePhysics(m_Registry, player, m_RigidBodyWorld2D);

    static MarioGameSensorCallback jumpCallback = [](Registry* registry, void* entity, Physics::ContactListener::ContactState contactState,
                                                     [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        static U32 collisionsCount = 0;
        Entity e;
        e.Id = U32(entity);
        switch (contactState)
        {
        case Physics::ContactListener::ContactState::Begin:
            collisionsCount++;
            registry->Get<Component::MarioInput>(e).CanJump = true;
            break;
        case Physics::ContactListener::ContactState::End:
            collisionsCount--;
            if (collisionsCount == 0)
                registry->Get<Component::MarioInput>(e).CanJump = false;
            break;
        }
    };
    footSensor.PhysicsCollider->SetUserData(jumpCallback);

    m_Registry.Add<Component::SpriteRenderer>(player, Component::SpriteRenderer(
        m_MarioSprites.get(),
        m_MarioSprites->GetSubTexturePixelsUV({216, 410}, {15, 28}),
        glm::vec4{1.0f},
        glm::vec2{1.0f, 1.0f},
        m_SortingLayer.GetLayer("Middleground"),
        1
    ));

    auto& anim = m_Registry.Add<Component::Animation>(player);
    anim.SpriteAnimation = m_Animations.front().get();

}

void MarioScene::CreateLevel()
{
    // Add background
    {
        Entity background = m_Registry.CreateEntity("background");
        auto& tf = m_Registry.Add<Component::Transform2D>(background);
        tf.Scale = {80.0f, 15.0f};
        auto& sr = m_Registry.Add<Component::SpriteRenderer>(background);
        sr.Tint = {0.52f, 0.80f, 0.92f, 1.0f};
        sr.SortingLayer = m_SortingLayer.GetLayer("Background");
        sr.OrderInLayer = -1;
    }
    {
        Entity floor = m_Registry.CreateEntity("level");
        auto& tf = m_Registry.Add<Component::Transform2D>(floor);
        auto& rb = m_Registry.Add<Component::RigidBody2D>(floor);
        auto& col = m_Registry.Add<Component::BoxCollider2D>(floor);
        tf.Position = glm::vec2{0.0f, -6.0f};
        tf.Scale = glm::vec2{20.0f, 1.0f};
        SceneUtils::SynchronizePhysics(m_Registry, floor, m_RigidBodyWorld2D);
        m_Registry.Add<Component::SpriteRenderer>(floor, Component::SpriteRenderer(
            m_BrickTexture.get(),
            std::array<glm::vec2, 4>{
                glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f}
            },
            glm::vec4{1.0f},
            glm::vec2{20.0f, 1.0f},
            m_SortingLayer.GetLayer("Middleground"),
            0
        ));
    }

    // Smaller platforms.
    {
        Entity floor = m_Registry.CreateEntity("level");
        auto& tf = m_Registry.Add<Component::Transform2D>(floor);
        auto& rb = m_Registry.Add<Component::RigidBody2D>(floor);
        auto& col = m_Registry.Add<Component::BoxCollider2D>(floor);
        tf.Position = glm::vec2{2.0f, -2.0f};
        tf.Scale = glm::vec2{2.0f, 1.0f};
        SceneUtils::SynchronizePhysics(m_Registry, floor, m_RigidBodyWorld2D);
        m_Registry.Add<Component::SpriteRenderer>(floor, Component::SpriteRenderer(
            m_BrickTexture.get(),
            std::array<glm::vec2, 4>{
                glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f}
            },
            glm::vec4{1.0f},
            glm::vec2{2.0f, 1.0f},
            m_SortingLayer.GetLayer("Middleground"),
            0
        ));
    }
    {
        Entity floor = m_Registry.CreateEntity("level");
        auto& tf = m_Registry.Add<Component::Transform2D>(floor);
        auto& rb = m_Registry.Add<Component::RigidBody2D>(floor);
        auto& col = m_Registry.Add<Component::BoxCollider2D>(floor);
        tf.Position = glm::vec2{3.0f, -3.0f};
        tf.Scale = glm::vec2{2.0f, 1.0f};
        SceneUtils::SynchronizePhysics(m_Registry, floor, m_RigidBodyWorld2D);
        m_Registry.Add<Component::SpriteRenderer>(floor, Component::SpriteRenderer(
            m_BrickTexture.get(),
            std::array<glm::vec2, 4>{
                glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f}
            },
            glm::vec4{1.0f},
            glm::vec2{2.0f, 1.0f},
            m_SortingLayer.GetLayer("Middleground"),
            0
        ));
    }
}

void MarioScene::FindActiveEntity()
{
    ENGINE_ASSERT(m_FrameBuffer != nullptr, "Framebuffer is unset")

    if (!m_FindActiveEntity) return;
    m_FindActiveEntity = false;
    // Read id from framebuffer texture (should only be executed in "Editor" mode.

    // First check that mouse is in main viewport, because imgui windows outside have
    // entity id of 0, instead of "clear" id of -1.
    glm::vec2 mousePos = Input::MousePosition();
    if (mousePos.x < 0.0f || mousePos.x > ImguiState::MainViewportSize.x ||
        mousePos.y < 0.0f || mousePos.y > ImguiState::MainViewportSize.y)
    {
        return;
    }
    // Read id texture, and get entityId from it.
    I32 entityId = m_FrameBuffer->ReadPixel(1, static_cast<U32>(mousePos.x), static_cast<U32>(mousePos.y),
                                            RendererAPI::DataType::Int);
    m_ActiveEntity = entityId;
}

bool MarioScene::OnMousePressed(MouseButtonPressedEvent& event)
{
    // Delay actual finding until framebuffer is bound.
    m_FindActiveEntity = true;
    return false;
}
