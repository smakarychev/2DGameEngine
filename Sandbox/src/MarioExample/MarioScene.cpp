#include "MarioScene.h"

#include "MarioActions.h"
#include "MarioComponentSerializers.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/SceneUtils.h"

void MarioScene::OnInit()
{
    InitSensorCallbacks();
    m_SceneSerializer.AddComponentSerializer<SensorsSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioInputSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioStateSerializer>();
    m_SceneSerializer.AddComponentSerializer<CollisionCallbackSerializer>();
    
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
    m_ContactListener.SetSensorCallbacks(&m_SensorCallbacks);

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
    m_AnimationsMap["wRight"] = walkRight;
    m_AnimationsMap["wLeft"] = walkLeft;
    m_AnimationsMap["jRight"] = jumpRight;
    m_AnimationsMap["jLeft"] = jumpLeft;
    m_AnimationsMap["stand"] = stand;
}

void MarioScene::OnUpdate(F32 dt)
{
    if (!m_IsSceneReady) return;
    
    // Call systems.
    SMove();
    SPhysics(dt);
    SState();
    SAnimation(dt);

    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraController->OnUpdate(dt);
    camera->CameraFrameBuffer->Bind();
    m_SceneSerializer.OnCopyPaste();
    m_ScenePanels.OnUpdate();
    m_SceneGraph.OnUpdate();
    camera->CameraFrameBuffer->Unbind();
    SCamera();
}

void MarioScene::OnEvent(Event& event)
{
    if (!m_IsSceneReady) return;
    
    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraController->OnEvent(event);
    m_ScenePanels.OnEvent(event);
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(MarioScene::OnMousePressed));
}

void MarioScene::OnRender()
{
    if (!m_IsSceneReady) return;
        
    const auto& sortingLayer = Renderer2D::GetSortingLayer();
    Renderer2D::SetSortingLayer(m_SortingLayer);
    SRender();
    Renderer2D::SetSortingLayer(sortingLayer);
}

void MarioScene::OnImguiUpdate()
{
    m_ScenePanels.OnMainMenuDraw();

    glm::vec2 mousePos = Input::MousePosition();
    
    auto* camera = GetMainCamera();
    FrameBuffer* frameBuffer = nullptr;
    Entity entityUnderMouse = NULL_ENTITY;
    if (camera)
    {
        frameBuffer = camera->CameraFrameBuffer.get();
        frameBuffer->Bind();
        if (SceneUtils::HasEntityUnderMouse(mousePos, frameBuffer)) entityUnderMouse = SceneUtils::GetEntityUnderMouse(mousePos, frameBuffer);
        frameBuffer->Unbind();
        mousePos = camera->CameraController->GetCamera()->ScreenToWorldPoint(mousePos);
        mousePos = Math::SnapToGrid(mousePos, {1.0f, 1.0f});
    }
    
    m_MainViewportSize = ImguiMainViewportAcceptDnD(frameBuffer, "Viewport", [&](const ImGuiPayload* payload)
    {
        m_SceneSerializer.OnImguiPayloadAccept(payload, {.MousePos = mousePos, .EntityUnderMouse = entityUnderMouse});
    });
    
    if (!m_IsSceneReady) return;
    
    m_ScenePanels.OnImguiUpdate();
}

void MarioScene::OnSceneGlobalUpdate()
{
    m_SceneGraph.OnUpdate();
    SceneUtils::SynchronizePhysics(*this);
    m_ScenePanels.ResetActiveEntity();
}

void MarioScene::Open(const std::string& filename)
{
    // Clear current scene.
    Clear();
    // Open (deserialize new scene).
    m_SceneSerializer.Deserialize("assets/scenes/" + filename + ".scene");

    OnSceneGlobalUpdate();
    m_IsSceneReady = true;
}

void MarioScene::Save(const std::string& filename)
{
    // Serialize scene.
    m_SceneSerializer.Serialize("assets/scenes/" + filename + ".scene");
}

void MarioScene::Clear()
{
    Renderer2D::Reset();
    m_Registry.Clear();
    m_RigidBodyWorld2D.Clear();
}

void MarioScene::PerformAction(Action& action)
{
    action.Execute();
}

void MarioScene::SPhysics(F32 dt)
{
    SceneUtils::PreparePhysics(*this);
    m_RigidBodyWorld2D.Update(dt);
    for (auto e : View<Component::RigidBody2D>(m_Registry))
    {
        SceneUtils::SynchronizeWithPhysics(*this, e);
    }
    for (auto e : View<Component::RigidBody2D, Component::LocalToParentTransform2D>(m_Registry))
    {
        SceneUtils::SynchronizeWithPhysicsLocalTransforms(*this, e);
    }
}

void MarioScene::SRender()
{
    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraFrameBuffer->Bind();
    RenderCommand::ClearScreen();
    I32 clearInteger = static_cast<I32>(NULL_ENTITY);
    camera->CameraFrameBuffer->ClearAttachment(1, RendererAPI::DataType::Int, &clearInteger);
    Renderer2D::BeginScene(camera->CameraController->GetCamera().get());

    for (auto e : View<Component::SpriteRenderer>(m_Registry))
    {
        const U32 entityId = e.Id;
        auto& tf = m_Registry.Get<Component::LocalToWorldTransform2D>(e);
        auto& sr = m_Registry.Get<Component::SpriteRenderer>(e);
        Renderer2D::DrawQuadEditor(entityId, tf, sr);
    }

    // TODO: toggle debug draw.
    RigidBodyWorldDrawer::Draw(m_RigidBodyWorld2D);
    //BVHTreeDrawer::Draw(m_RigidBodyWorld2D.GetBroadPhase().GetBVHTree());
    
    Renderer2D::EndScene();
    camera->CameraFrameBuffer->Unbind();
    ValidateViewport();
}

void MarioScene::SMove()
{
    static constexpr auto MAX_HORIZONTAL_SPEED = 6.0f;
    for (auto e : View<Component::MarioInput, Component::RigidBody2D>(m_Registry))
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
    for (auto e : View<Component::Animation, Component::SpriteRenderer>(m_Registry))
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

    for (auto e : View<Component::Animation, Component::MarioState>(m_Registry))
    {
        auto& animation = m_Registry.Get<Component::Animation>(e);
        const auto& state = m_Registry.Get<Component::MarioState>(e);
        if (state.IsMovingLeft)
        {
            if (state.IsInMidAir && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["jLeft"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["jLeft"]);
            }
            else if (!state.IsInMidAir && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wLeft"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wLeft"]);
            }
        }
        else if (state.IsMovingRight)
        {
            if (state.IsInMidAir && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["jRight"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["jRight"]);
            }
            else if (!state.IsInMidAir && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wRight"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wRight"]);
            }
        }
        else
        {
            animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["stand"]);
        }
    }
}

void MarioScene::SState()
{
    for (auto e : View<Component::RigidBody2D, Component::MarioState>(m_Registry))
    {
        // Update state based on rigid body characteristics.
        const auto& rb = m_Registry.Get<Component::RigidBody2D>(e);
        auto& state = m_Registry.Get<Component::MarioState>(e);
        const auto& vel = rb.PhysicsBody->GetLinearVelocity();
        state.IsMovingLeft = vel.x < -0.01f;
        state.IsMovingRight = vel.x > 0.01f;
        state.IsInFreeFall = vel.y < -0.01f;
        state.IsInMidAir = state.IsInFreeFall || vel.y > 0.01f;
    }
}

void MarioScene::SCamera()
{
    for (auto cE : View<Component::Camera>(m_Registry))
    {
        auto& camera = m_Registry.Get<Component::Camera>(cE);
        auto& tf = m_Registry.Get<Component::LocalToWorldTransform2D>(cE);
        tf.Position = camera.CameraController->GetCamera()->GetPosition();
    }
}

void MarioScene::InitSensorCallbacks()
{
    Component::CollisionCallback::SensorCallback jumpCallback = [](
        Registry* registry, const Component::CollisionCallback::CollisionData& collisionData,
        [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        auto& collisionCallback = registry->Get<Component::CollisionCallback>(collisionData.Primary);
        Entity parent = registry->Get<Component::ParentRel>(collisionData.Primary).Parent;
        switch (collisionData.ContactState)
        {
        case Physics::ContactListener::ContactState::Begin:
            collisionCallback.CollisionCount++;
            registry->Get<Component::MarioInput>(parent).CanJump = true;
            break;
        case Physics::ContactListener::ContactState::End:
            collisionCallback.CollisionCount--;
            if (collisionCallback.CollisionCount == 0)
                registry->Get<Component::MarioInput>(parent).CanJump = false;
            break;
        }
    };
    m_SensorCallbacks.push_back(jumpCallback);
}

void MarioScene::ValidateViewport()
{
    auto* camera = GetMainCamera();
    auto& framebuffer = camera->CameraFrameBuffer;
    if (FrameBuffer::Spec spec = framebuffer->GetSpec();
        m_MainViewportSize.x > 0.0f && m_MainViewportSize.y > 0.0f &&
        (spec.Width != m_MainViewportSize.x || spec.Height != m_MainViewportSize.y))
    {
        framebuffer->Resize((U32)m_MainViewportSize.x, (U32)m_MainViewportSize.y);
        camera->CameraController->GetCamera()->SetViewport((U32)m_MainViewportSize.x, (U32)m_MainViewportSize.y);
        RenderCommand::SetViewport((U32)m_MainViewportSize.x, (U32)m_MainViewportSize.y);
    }
}

Component::Camera* MarioScene::GetMainCamera()
{
    for (auto cameraE : View<Component::Camera>(m_Registry))
    {
        auto& camera = m_Registry.Get<Component::Camera>(cameraE);
        if (camera.IsPrimary) return &camera;
    }
    return nullptr;
}

FrameBuffer* MarioScene::GetMainFrameBuffer()
{
    auto* camera = GetMainCamera();
    if (!camera) return nullptr;
    return GetMainCamera()->CameraFrameBuffer.get();
}


bool MarioScene::OnMousePressed(MouseButtonPressedEvent& event)
{
    return false;
}
