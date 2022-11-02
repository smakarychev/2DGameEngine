#include "enginepch.h"

#include "Scene.h"

#include "Engine/Core/Input.h"
#include "Engine/Rendering/Drawers/BVHTreeDrawer.h"
#include "Engine/Rendering/Drawers/RigidBodyWorldDrawer.h"
#include "Engine/Imgui/ImguiCommon.h"

namespace Engine
{
    
    /*Entity& Scene::CreateEntity(const std::string& tag)
    {
        Entity& e = m_Manager.AddEntity(tag);
        e.AddComponent<Component::Transform2D>(glm::vec2{ 0.0f }, glm::vec2{ 1.0f }, glm::vec2{ 1.0f, 0.0f });
        return e;
    }

    void Scene::OnUpdate(F32 dt)
    {
        m_Manager.Update();
        m_World.Update(dt);
        for (auto& e : m_Manager.GetEntities())
        {
            if (e->HasComponent<Component::RigidBody2D>())
            {
                auto& rbody = e->GetComponent<Component::RigidBody2D>().PhysicsBody;
                auto& transform = e->GetComponent<Component::Transform2D>();
                transform.Position = rbody->GetPosition();
                transform.Rotation = rbody->GetRotation();
            }
        }
        if (m_FindEntity)
        {
            FindActiveEntity();
            m_FindEntity = false;
        }
        OnActiveEntityUIUpdate();
        SynchronizeActiveEntityState();
    }

    void Scene::OnInit()
    {
        // Add all entities.
        m_Manager.Update();
        for (auto& e : m_Manager.GetEntities())
        {
            auto& transform = e->GetComponent<Component::Transform2D>();
            if (e->HasComponent<Component::RigidBody2D>())
            {
                auto& rBody = e->GetComponent<Component::RigidBody2D>();
                rBody.PhysicsBody->SetPosition(transform.Position);
                rBody.PhysicsBody->SetRotation(transform.Rotation);
                rBody.PhysicsBody->SetType(rBody.Type);
                rBody.PhysicsBody->SetFlags(rBody.Flags);

                auto& collider = e->GetComponent<Component::BoxCollider2D>();
                collider.HalfSize *= Math::Abs(transform.Scale);
                collider.PhysicsCollider->HalfSize = collider.HalfSize;
                collider.PhysicsCollider->SetSensor(collider.IsSensor);
                collider.PhysicsCollider->Center = collider.Offset;
                collider.PhysicsCollider->SetPhysicsMaterial(collider.PhysicsMaterial);

                if (rBody.Type == Physics::RigidBodyType2D::Dynamic) rBody.PhysicsBody->RecalculateMass();
            }
        }
    }

    void Scene::FindActiveEntity()
    {
        glm::vec2 mousePos = Input::MousePosition();
        // TODO: currently imgui has 0 as entityId (for some reason), so we first check if click is withing the viewport.
        if (mousePos.x < 0.0f || mousePos.y < 0.0f) return;
        
        // Find entityId beneath mouse.
        const I32 entityId = m_FrameBuffer->ReadPixel(1, static_cast<U32>(mousePos.x), static_cast<U32>(mousePos.y), RendererAPI::DataType::Int);
        if (entityId != -1)
        {
            m_ActiveEntity = m_Manager.GetEntityById(entityId);
        }
        mousePos = m_Camera->ScreenToWorldPoint(mousePos);
    }
    
    void Scene::OnActiveEntityUIUpdate()
    {
        ImGui::Begin("Inspector");
        if (m_ActiveEntity != nullptr)
        {
            auto& transform = m_ActiveEntity->GetComponent<Component::Transform2D>();
            F32 rotationDegrees = glm::degrees(std::acos(transform.Rotation[0]));
            if (transform.Rotation[1] < 0.0f)  rotationDegrees *= -1.0f;
            ImGui::Begin("Transform2D");
            ImGui::DragFloat2("Position", &transform.Position[0], 0.25f, -10.0f, 10.0f);
            ImGui::DragFloat("Rotation", &rotationDegrees, 0.25f, -360.0f, 360.0f);
            ImGui::DragFloat2("Scale", &transform.Scale[0], 0.25f, -10.0f, 10.0f);
            ImGui::End();

            const F32 rotationRadians = glm::radians(rotationDegrees);
            transform.Rotation = glm::vec2{ glm::cos(rotationRadians), glm::sin(rotationRadians) };
        }
        ImGui::End();
        
    }

    void Scene::SynchronizeActiveEntityState()
    {
        if (m_ActiveEntity == nullptr) return;
        if (m_ActiveEntity->HasComponent<Component::RigidBody2D>())
        {
            auto& tf = m_ActiveEntity->GetComponent<Component::Transform2D>();
            auto& rb = m_ActiveEntity->GetComponent<Component::RigidBody2D>();
            auto& col = m_ActiveEntity->GetComponent<Component::BoxCollider2D>();
            rb.PhysicsBody->SetPosition(tf.Position);
            rb.PhysicsBody->SetRotation(tf.Rotation);
            col.HalfSize = glm::vec2{ 0.5f } * Math::Abs(tf.Scale);
            col.PhysicsCollider->HalfSize = col.HalfSize;
        }
    }

    void Scene::AddDefaultRigidBodyComponent(Entity& entity)
    {
        auto& rBody = entity.AddComponent<Component::RigidBody2D>();
        rBody.Type = Physics::RigidBodyType2D::Static;
        rBody.Flags = Physics::RigidBodyDef2D::BodyFlags::RestrictRotation;
        Physics::RigidBodyDef2D rbDef;
        rbDef.UserData = (void*)(&entity);
        rBody.PhysicsBody = m_World.CreateBody(rbDef);

        auto& collider = entity.AddComponent<Component::BoxCollider2D>();
        collider.HalfSize = glm::vec2{ 0.5f };
        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(collider.Offset, collider.HalfSize);
        colDef.Collider = &box;
        collider.PhysicsCollider = (Physics::BoxCollider2D*)m_World.AddCollider(rBody.PhysicsBody, colDef);
    }

    Physics::BoxCollider2D* Scene::AddDefaultBoxCollider(Entity& entity)
    {
        // Add all entities.
        m_Manager.Update();
        if (entity.HasComponent<Component::RigidBody2D>() == false) return nullptr;
        auto& transform = entity.GetComponent<Component::Transform2D>();

        Physics::ColliderDef2D colDef;
        Physics::BoxCollider2D box = Physics::BoxCollider2D(glm::vec2{ 0.0f }, glm::vec2{ 0.5f } * Math::Abs(transform.Scale));
        colDef.Collider = &box;
        return static_cast<Physics::BoxCollider2D*>(m_World.AddCollider(entity.GetComponent<Component::RigidBody2D>().PhysicsBody, colDef));
    }

    void Scene::DebugDraw()
    {
        RigidBodyWorldDrawer::Draw(m_World);
        //BVHTreeDrawer<AABB2D>::Draw(m_World.GetBroadPhase().GetBVHTree());
        //BVHTreeDrawer<AABB2D>::Draw(m_SceneBoundingTree);
    }

    void Scene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(Scene::OnMouseButtonPressed));
    }

    void Scene::OnRender()
    {
        
    }

    bool Scene::OnMouseButtonPressed(MouseButtonPressedEvent& event)
    {
        // Delay finding the entity until we know that framebuffer is bound.
        m_FindEntity = true;
        return false;
    }*/

}
