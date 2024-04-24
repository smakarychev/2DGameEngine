#include "MarioScene.h"

#include "Controllers/MarioCameraController.h"
#include "MarioComponentSerializers.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/SceneUtils.h"

MarioScene::MarioScene()
    : 
    m_PlayerFsm(*this), m_GoombaFsm(*this),
    m_PiranhaPlantFsm(*this), m_KoopaFsm(*this),
    m_BlockFsm(*this)
{
}

void MarioScene::OnInit()
{
    m_PlayerFsm.ReadConfig("assets/configs/PlayerFSM.yaml");
    m_GoombaFsm.ReadConfig("assets/configs/GoombaFSM.yaml");
    m_PiranhaPlantFsm.ReadConfig("assets/configs/PiranhaPlantFSM.yaml");
    m_KoopaFsm.ReadConfig("assets/configs/KoopaFSM.yaml");
    m_BlockFsm.ReadConfig("assets/configs/BlockFSM.yaml");

    m_SceneSerializer.AddComponentSerializer<SensorsSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioMenuItemSerializer>();

    REGISTER_TAG_COMPONENT(MarioPlayerTag)
    REGISTER_TAG_COMPONENT(MarioLevelTag)
    REGISTER_TAG_COMPONENT(MarioEmptyBlockTag)
    REGISTER_TAG_COMPONENT(MarioCoinBlockTag)
    REGISTER_TAG_COMPONENT(MarioCoinTag)
    REGISTER_TAG_COMPONENT(MarioEnemyTag)
    REGISTER_TAG_COMPONENT(MarioGoombaTag)
    REGISTER_TAG_COMPONENT(MarioPiranhaPlantTag)
    REGISTER_TAG_COMPONENT(MarioKoopaTag)
    REGISTER_TAG_COMPONENT(MarioAwakeTag)
    REGISTER_TAG_COMPONENT(MarioKillTag)
    REGISTER_TAG_COMPONENT(MarioWinTag)

    
    // Create sorting layer for correct render order.
    // The structure is Background->Middleground->Default.
    m_SortingLayer.CreateLayer("Background");
    m_SortingLayer.CreateLayer("Middleground");
    m_SortingLayer.PlaceBefore(m_SortingLayer.GetLayer("Middleground"), m_SortingLayer.GetLayer("Default"));
    m_SortingLayer.PlaceBefore(m_SortingLayer.GetLayer("Background"), m_SortingLayer.GetLayer("Middleground"));

    // Adjust gravity for better feel.
    m_RigidBodyWorld2D.SetGravity({0.0f, -50.0f});
    // Set custom contact listener.
    m_RigidBodyWorld2D.SetContactListener(&m_ContactListener);
    m_ContactListener.SetRegistry(&m_Registry);
    m_ContactListener.SetSensorCallbacks(&m_SensorCallbacks);

    // Load assets.
    // Pixel fonts dont like linear filter.
    m_Font = Font::ReadFontFromFile("assets/fonts/WHITRABT.ttf");

    Open(m_DefaultScene);
}

void MarioScene::OnScenePlay()
{
    m_IsPlaying = true;
}

void MarioScene::OnSceneStop()
{
    m_IsPlaying = false;
}

void MarioScene::OnUpdate(F32 dt)
{
    if (!m_IsSceneReady) return;

    if (m_IsPlaying)
    {
        // Call systems.
        m_PlayerFsm.OnUpdate(dt);
        m_GoombaFsm.OnUpdate(dt);
        m_PiranhaPlantFsm.OnUpdate(dt);
        m_KoopaFsm.OnUpdate(dt);
        m_BlockFsm.OnUpdate(dt);
        SKill(dt);
        SPhysics(dt);
        SAnimation(dt);
        SGameState();
    }
    
    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraController->OnUpdate(dt);
    RenderEditor();
    
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

void MarioScene::OnSceneGlobalUpdate(Entity addedEntity)
{
    InitEntities();
    m_SceneGraph.UpdateGraphOfEntity(addedEntity);
    SceneUtils::TraverseTree(addedEntity, m_Registry, [&](Entity e)
    {
        if (m_Registry.Has<Component::BoxCollider2D>(e)) SceneUtils::SynchronizePhysics(*this, e, SceneUtils::PhysicsSynchroSetting::ColliderOnly);
        if (m_Registry.Has<Component::RigidBody2D>(e)) SceneUtils::SynchronizePhysics(*this, e, SceneUtils::PhysicsSynchroSetting::RBOnly);
    });
    m_ScenePanels.ResetActiveEntity();
}

void MarioScene::Open(const std::string& filename)
{
    std::string path = filename;
    m_CurrentScene = path;
    // Clear current scene.
    Clear();
    // Open (deserialize new scene).
    m_SceneSerializer.Deserialize("assets/scenes/" + path + ".scene");
    
    InitEntities();
    m_SceneGraph.OnUpdate();
    SceneUtils::SynchronizePhysics(*this);
    m_ScenePanels.ResetActiveEntity();
    SceneUtils::SynchronizeCamerasWithTransforms(*this);
    m_IsSceneReady = true;
    InitGameWinCollisionCallback();
    
    m_GameState = GameState::Running;
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
    m_Player = NULL_ENTITY;
}

void MarioScene::SGameState()
{
    if (!m_Registry.IsEntityExists(m_Player)) m_Player = NULL_ENTITY;
    switch (m_GameState)
    {
    case GameState::Menu: break;
    case GameState::Running:
        {
            if (Input::GetKeyDown(Key::Escape))
            {
                m_GameState = GameState::Menu; Open("Game menu");
            }
            if (m_Player)
            {
                if (m_Registry.Has<MarioHarmlessTag>(m_Player)) m_GameState = GameState::GameOver;
                else if (m_Registry.Has<MarioWinTag>(m_Player)) m_GameState = GameState::GameWin;
            }
            break;
        }
    case GameState::GameOver:
    case GameState::GameWin:
        {
            if (Input::GetKeyDown(Key::Escape))
            {
                m_GameState = GameState::Menu; Open("Game menu");
            }
            break;
        }
    }
}

void MarioScene::SGameMenu()
{
    static U32 selectedMenuItemIndex = 0;
    
    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.FontSize = 36.0f;
    fr.SortingLayer = m_SortingLayer.GetLayer("Middleground"); fr.OrderInLayer = 999;
    fr.Zoom = 8;
    fr.FontSize = 36.0f;
    U32 menuItemsCount = 0;
    for (auto e : View<MarioMenuItem>(m_Registry))
    {
        menuItemsCount++;
        auto& menuItem = m_Registry.Get<MarioMenuItem>(e);
        
        if (menuItem.Order == selectedMenuItemIndex) fr.Tint = {0.1f, 0.1f, 0.0f, 1.0f};
        else fr.Tint = {0.9f, 0.9f, 1.0f, 1.0f};
        
        fr.FontRect = {.Min = {10.0f, 10.0f + static_cast<F32>(1 + menuItem.Order) * fr.FontSize * 2.5f}, .Max = m_MainViewportSize};
        Renderer2D::DrawFontFixedEditor(e, fr, menuItem.Title);

        if (Input::GetKeyDown(Key::Enter) && menuItem.Order == selectedMenuItemIndex)
        {
            switch (menuItem.Type)
            {
            case MarioMenuItem::Type::Exit: Application::Get().Exit(); break;
            case MarioMenuItem::Type::Open: Open(menuItem.Info); break;
            }
            break;
        }
    }
    if (menuItemsCount == 0) return;
    bool down = Input::GetKeyDown(Key::S) || Input::GetKeyDown(Key::Down);
    bool up = Input::GetKeyDown(Key::W) || Input::GetKeyDown(Key::Up);
    if (down) selectedMenuItemIndex = (selectedMenuItemIndex + 1) % menuItemsCount;
    if (up) selectedMenuItemIndex = (selectedMenuItemIndex - 1 + menuItemsCount) % menuItemsCount;
}


void MarioScene::SKill(F32 dt)
{
    if (m_Player == NULL_ENTITY || m_Registry.Has<Component::LifeTimeComponent>(m_Player)) m_GameState = GameState::GameOver;
    
    for (auto e : View<Component::LifeTimeComponent>(m_Registry))
    {
        auto& killComponent = m_Registry.Get<Component::LifeTimeComponent>(e);
        killComponent.LifeTimeLeft -= static_cast<F64>(dt);
        if (killComponent.LifeTimeLeft < 0.0)
        {
            if (m_Player != NULL_ENTITY && m_Registry.Has<Component::ScoreComponent>(e))
            {
                auto& playerScore = m_Registry.Get<Component::ScoreComponent>(m_Player);
                playerScore.Score += m_Registry.Get<Component::ScoreComponent>(e).Score;
            }
            SceneUtils::DeleteEntity(*this, e);
        }
    }
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
        SceneUtils::SynchronizeWithPhysicsLocal(*this, e);
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
        auto& tf = m_Registry.Get<Component::LocalToWorldTransform2D>(e);
        auto& sr = m_Registry.Get<Component::SpriteRenderer>(e);
        Renderer2D::DrawQuadEditor(e.Id, tf, sr);
    }

    SRenderText();
    SGameMenu();

    // TODO: toggle debug draw.
    RigidBodyWorldDrawer::Draw(m_RigidBodyWorld2D);
    BVHTreeDrawer::Draw(m_RigidBodyWorld2D.GetBroadPhase().GetBVHTree());
    
    Renderer2D::EndScene();
    camera->CameraFrameBuffer->Unbind();
    ValidateViewport();
}

void MarioScene::SRenderText()
{
    // Draw points (score).
    for (auto e : View<Component::LifeTimeComponent, Component::ScoreComponent>(m_Registry))
    {
        if (m_Registry.Has<MarioPlayerTag>(e)) continue;
        
        auto& lifeTime = m_Registry.Get<Component::LifeTimeComponent>(e);
        auto& score = m_Registry.Get<Component::ScoreComponent>(e);
        auto& tf = m_Registry.Get<Component::LocalToWorldTransform2D>(e);
        auto& fr = m_Registry.Get<Component::FontRenderer>(e);
        fr.Font = m_Font.get();
        fr.SortingLayer = m_SortingLayer.GetLayer("Middleground"); fr.OrderInLayer = 999;
        fr.FontRect.Min.y = tf.Position.y +
            static_cast<F32>(Math::Lerp(
                0.0f,
                2.0f * tf.Scale.y,
                Math::ILerp(lifeTime.LifeTime, 0.0f, lifeTime.LifeTimeLeft
            )
        ));
        Renderer2D::DrawFontEditor(e.Id, fr, std::format("{}", score.Score));
    }
    
    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.Tint = {0.9f, 0.9f, 1.0f, 1.0f};
    fr.FontSize = 36.0f;
    fr.SortingLayer = m_SortingLayer.GetLayer("Middleground"); fr.OrderInLayer = 999;
    fr.Zoom = 8;
    fr.FontRect = {.Min = {10.0f, 10.0f}, .Max = m_MainViewportSize};
    if (m_Player == NULL_ENTITY) return;
    switch (m_GameState)
    {
    case GameState::Running:
    {
        auto& score = m_Registry.Get<Component::ScoreComponent>(m_Player);
        Renderer2D::DrawFontFixedEditor(NULL_ENTITY, fr, std::format("Score: {}", score.Score));
        break;
    }
    case GameState::GameOver:
    {
        fr.FontSize = 48.0f;
        static U64 totalScore = m_Registry.Get<Component::ScoreComponent>(m_Player).Score;
        Renderer2D::DrawFontFixedEditor(NULL_ENTITY, fr, std::format("Game over! Total score: {}", totalScore));
        break;
    }
    case GameState::GameWin:
    {
        fr.FontSize = 48.0f;
        static U64 totalScore = m_Registry.Get<Component::ScoreComponent>(m_Player).Score;
        Renderer2D::DrawFontFixedEditor(NULL_ENTITY, fr, std::format("You win! Total score: {}", totalScore));
        break;
    }
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
        sr.Texture = animation.SpriteAnimation->GetSpriteSheet();
    }
}

void MarioScene::SCamera()
{
    for (auto cE : View<Component::Camera>(m_Registry))
    {
        auto& camera = m_Registry.Get<Component::Camera>(cE);
        auto& tf = m_Registry.Get<Component::LocalToWorldTransform2D>(cE);
        tf.Position = camera.CameraController->GetCamera()->GetPosition();
        if (m_Registry.Has<Component::LocalToParentTransform2D>(cE))
        {
            Entity parent = m_Registry.Get<Component::ParentRel>(cE).Parent;
            m_Registry.Get<Component::LocalToParentTransform2D>(cE) =
                tf.Concatenate(m_Registry.Get<Component::LocalToWorldTransform2D>(parent).Inverse());
        }
    }
}


void MarioScene::AddSensorCallback(const std::string& callbackName,
                                   CollisionCallback::SensorCallback callback)
{
    m_SensorCallbacks[callbackName] = callback;
}

void MarioScene::InitEntities()
{
    InitPlayer();
    InitGoomba();
    InitPiranhaPlant();
    InitKoopa();
    InitBlock();
    InitCameraController();
}

void MarioScene::InitPlayer()
{
    // We serialize the minimum amount of components, and instead fill the rest in functions like that.
    for (auto e : View<MarioPlayerTag>(m_Registry))
    {
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& topCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Top);
        auto& bottomCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Bottom);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        rightCallback.CallbackName = leftCallback.CallbackName = "Player";
        topCallback.CallbackName = bottomCallback.CallbackName = "Player";

        auto& score = m_Registry.AddOrGet<Component::ScoreComponent>(e);
        score.Score = 0;
        m_PlayerFsm.RegisterEntity(e);
        m_Player = e;
    }
}

void MarioScene::InitGoomba()
{
    for (auto e : View<MarioGoombaTag>(m_Registry))
    {
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        auto& topCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Top);
        leftCallback.CallbackName = rightCallback.CallbackName = topCallback.CallbackName = "Goomba";
        m_GoombaFsm.RegisterEntity(e);
    }
}

void MarioScene::InitPiranhaPlant()
{
    for (auto e : View<MarioPiranhaPlantTag>(m_Registry))
    {
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& topCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Top);
        topCallback.CallbackName = "PiranhaPlant";
        m_PiranhaPlantFsm.RegisterEntity(e);
    }
}

void MarioScene::InitKoopa()
{
    for (auto e : View<MarioKoopaTag>(m_Registry))
    {
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        auto& topCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Top);

        leftCallback.CallbackName = rightCallback.CallbackName = topCallback.CallbackName = "Koopa";
        m_KoopaFsm.RegisterEntity(e);
    }
}

void MarioScene::InitBlock()
{
    for (auto e : View<MarioEmptyBlockTag>(m_Registry))
    {
        auto& callback = m_Registry.AddOrGet<CollisionCallback>(e);
        callback.CallbackName = "Block";
        m_BlockFsm.RegisterEntity(e);
    }
    for (auto e : View<MarioCoinBlockTag>(m_Registry))
    {
        auto& callback = m_Registry.AddOrGet<CollisionCallback>(e);
        callback.CallbackName = "Block";
        m_BlockFsm.RegisterEntity(e);
    }
    for (auto e : View<MarioCoinTag>(m_Registry))
    {
        auto& callback = m_Registry.AddOrGet<CollisionCallback>(e);
        callback.CallbackName = "Block";
        m_BlockFsm.RegisterEntity(e);
    }
}

void MarioScene::InitCameraController()
{
    auto* camera = GetMainCamera();
    if (!camera) return;
    if (camera->CameraController->GetControllerType() != CameraController::ControllerType::Custom) return;
    glm::vec3 cameraPos = camera->CameraController->GetCamera()->GetPosition();
    Ref<MarioCameraController> controller = CreateRef<MarioCameraController>(camera->CameraController->GetCamera(), *this);
    controller->ReadConfig("assets/configs/CameraController.yaml");
    controller->SetTarget(m_Player);
    camera->CameraController = controller;
    camera->CameraController->GetCamera()->SetPosition(cameraPos);
}

void MarioScene::InitGameWinCollisionCallback()
{
    auto winCollisionCallback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        Entity other = collisionData.Secondary;
        if (registry->Has<MarioPlayerTag>(other))
        {
            registry->AddOrGet<MarioWinTag>(other);
        }
    };
    AddSensorCallback("Win", winCollisionCallback);
    for (auto e : View<MarioWinTag>(m_Registry))
    {
        auto& colCallback = m_Registry.AddOrGet<CollisionCallback>(e);
        colCallback.CallbackName = "Win";
    }
}

void MarioScene::RenderEditor()
{
    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraFrameBuffer->Bind();
    m_SceneSerializer.OnCopyPaste();
    m_ScenePanels.OnUpdate();
    m_SceneGraph.OnUpdate();
    if (GetMainCamera()) camera->CameraFrameBuffer->Unbind();
}


void MarioScene::ValidateViewport()
{
    auto* camera = GetMainCamera();
    auto& framebuffer = camera->CameraFrameBuffer;
    if (FrameBuffer::Spec spec = framebuffer->GetSpec();
        m_MainViewportSize.x > 0.0f && m_MainViewportSize.y > 0.0f &&
        (static_cast<F32>(spec.Width) != m_MainViewportSize.x || static_cast<F32>(spec.Height) != m_MainViewportSize.y))
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