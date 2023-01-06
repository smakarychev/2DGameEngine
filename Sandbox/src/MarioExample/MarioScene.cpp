#include "MarioScene.h"

#include "Controllers/MarioCameraController.h"
#include "MarioComponentSerializers.h"
#include "Controllers/ControllersCommon.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/SceneUtils.h"

MarioScene::MarioScene()
    : m_PlayerController(*this), m_PlayerAnimator(*this),
    m_GoombaController(*this), m_GoombaAnimator(*this),
    m_PiranhaPlantController(*this), m_PiranhaPlantAnimator(*this),
    m_KoopaController(*this), m_KoopaAnimator(*this),
    m_PlayerFsm(*this)
{
}

void MarioScene::OnInit()
{
    m_PlayerController.ReadConfig("assets/configs/PlayerController.yaml");
    m_GoombaController.ReadConfig("assets/configs/GoombaController.yaml");
    m_PiranhaPlantController.ReadConfig("assets/configs/PiranhaPlantController.yaml");
    m_KoopaController.ReadConfig("assets/configs/KoopaController.yaml");
    m_PlayerAnimator.LoadAnimations("assets/configs/PlayerAnimator.yaml");
    m_GoombaAnimator.LoadAnimations("assets/configs/GoombaAnimator.yaml");
    m_PiranhaPlantAnimator.LoadAnimations("assets/configs/PiranhaPlantAnimator.yaml");
    m_KoopaAnimator.LoadAnimations("assets/configs/KoopaAnimator.yaml");

    m_PlayerFsm.ReadConfig("assets/configs/PlayerFSM.yaml");
    
    InitSensorCallbacks();
    m_SceneSerializer.AddComponentSerializer<SensorsSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioPlayerTagSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioLevelTagSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioEnemyTagSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioGoombaTagSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioPiranhaPlantTagSerializer>();
    m_SceneSerializer.AddComponentSerializer<MarioKoopaTagSerializer>();

    m_ScenePanels.AddComponentUiDesc<MarioPlayerTagUIDesc>();
    m_ScenePanels.AddComponentUiDesc<MarioLevelTagUIDesc>();
    m_ScenePanels.AddComponentUiDesc<MarioGoombaTagUIDesc>();
    m_ScenePanels.AddComponentUiDesc<MarioEnemyTagUIDesc>();
    m_ScenePanels.AddComponentUiDesc<MarioPiranhaPlantTagUIDesc>();
    m_ScenePanels.AddComponentUiDesc<MarioKoopaTagUIDesc>();
    
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
    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");
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
        //m_PlayerController.OnUpdate(dt);
        m_PlayerFsm.OnUpdate(dt);
        m_GoombaController.OnUpdate(dt);
        m_PiranhaPlantController.OnUpdate(dt);
        m_KoopaController.OnUpdate(dt);
        ControllerUtils::KillSystem(dt, *this);
        
        // Call systems.
        SPhysics(dt);
        SState();
        SAnimation(dt);

    }

    auto* camera = GetMainCamera();
    if (!camera) return;
    camera->CameraController->OnUpdate(dt);
    camera->CameraFrameBuffer->Bind();
    m_SceneSerializer.OnCopyPaste();
    m_ScenePanels.OnUpdate();
    m_SceneGraph.OnUpdate();
    if (GetMainCamera()) camera->CameraFrameBuffer->Unbind();
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
    InitPlayer();
    InitGoomba();
    InitPiranhaPlant();
    InitKoopa();
    InitCameraController();
    m_PlayerFsm.RegisterEntity(*View<MarioPlayerTag>(m_Registry).begin());
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
    SceneUtils::SynchonizeCamerasWithTransforms(*this);
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
    m_GoombaAnimator.OnUpdate();
    m_PiranhaPlantAnimator.OnUpdate();
    m_KoopaAnimator.OnUpdate();
}

void MarioScene::SState()
{
    for (auto e : View<Component::RigidBody2D, Component::MarioState>(m_Registry))
    {
        // Update state based on rigid body characteristics.
        const auto& rb = m_Registry.Get<Component::RigidBody2D>(e);
        auto& state = m_Registry.Get<Component::MarioState>(e);
        const auto& vel = rb.PhysicsBody->GetLinearVelocity();
        
        bool wasFacingLeft = state.IsFacingLeft;
        
        state.IsFacingLeft = vel.x < -0.01f;
        state.IsFacingRight = vel.x > 0.01f;

        if (!state.IsFacingLeft && !state.IsFacingRight)
        {
            if (wasFacingLeft) state.IsFacingLeft = true;
            else state.IsFacingRight = true;
        }
        
        state.IsMovingLeft = vel.x < -0.01f;
        state.IsMovingRight = vel.x > 0.01f;
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

void MarioScene::AddSensorCallback(const std::string& indexMajor, const std::string& indexMinor,
    CollisionCallback::SensorCallback callback)
{
    m_SensorCallbacks[indexMajor][indexMinor] = callback;
}

void MarioScene::InitPlayer()
{
    // We serialize the minimum amount of components, and instead fill the rest in functions like that.
    for (auto e : View<MarioPlayerTag>(m_Registry))
    {
        m_Registry.AddOrGet<Component::MarioState>(e);
        m_Registry.AddOrGet<Component::MarioInput>(e);
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& bottomCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Bottom);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        bottomCallback.IndexMajor = leftCallback.IndexMajor = rightCallback.IndexMajor = "Player";
        bottomCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Bottom).EntityName;
        leftCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Left).EntityName;
        rightCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Right).EntityName;
    }
}

void MarioScene::InitGoomba()
{
    for (auto e : View<MarioGoombaTag>(m_Registry))
    {
        m_Registry.AddOrGet<Component::GoombaState>(e);
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        leftCallback.IndexMajor = rightCallback.IndexMajor = "Goomba";
        leftCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Left).EntityName;
        rightCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Right).EntityName;
    }
    m_GoombaController.OnInit();
}

void MarioScene::InitPiranhaPlant()
{
    for (auto e : View<MarioPiranhaPlantTag>(m_Registry))
    {
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& topCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Top);
        topCallback.IndexMajor = "PiranhaPlant";
        topCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Top).EntityName;
    }
}

void MarioScene::InitKoopa()
{
    for (auto e : View<MarioKoopaTag>(m_Registry))
    {
        m_Registry.AddOrGet<Component::KoopaState>(e);
        auto& sensors = m_Registry.Get<Component::Sensors>(e);
        auto& leftCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Left);
        auto& rightCallback = m_Registry.AddOrGet<CollisionCallback>(sensors.Right);
        leftCallback.IndexMajor = rightCallback.IndexMajor = "Koopa";
        leftCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Left).EntityName;
        rightCallback.IndexMinor = m_Registry.Get<Component::Name>(sensors.Right).EntityName;
    }
    m_KoopaController.OnInit();
}

void MarioScene::InitCameraController()
{
    auto* camera = GetMainCamera();
    if (!camera) return;
    if (camera->CameraController->GetControllerType() != CameraController::ControllerType::Custom) return;
    glm::vec3 cameraPos = camera->CameraController->GetCamera()->GetPosition();
    Ref<MarioCameraController> controller = CreateRef<MarioCameraController>(camera->CameraController->GetCamera(), *this);
    controller->ReadConfig("assets/configs/CameraController.yaml");
    controller->SetTarget(*View<MarioPlayerTag>(m_Registry).begin());
    camera->CameraController = controller;
    camera->CameraController->GetCamera()->SetPosition(cameraPos);
}


bool MarioScene::OnMousePressed(MouseButtonPressedEvent& event)
{
    return false;
}
