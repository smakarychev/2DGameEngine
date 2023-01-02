#pragma once
#include "Engine/ECS/Components.h"
#include "Engine/ECS/Registry.h"

namespace Engine
{
    class Scene;
    class Registry;
}

namespace Engine
{
    class ComponentUIDescBase
    {
    public:
        using ComponentSignature = const char*;
    public:
        ComponentUIDescBase(const ComponentSignature& signature, bool isRemovable, Scene& scene);
        virtual ~ComponentUIDescBase();
        virtual void OnUIDraw(Entity e) = 0;
        virtual U64 GetComponentID() const = 0;
        virtual void RemoveComponent(Entity e) = 0;
        virtual bool ShouldDraw(Entity e) = 0;
        
        Registry& GetAttachedRegistry() const { return m_Registry; }
        const ComponentSignature& GetSignature() const { return m_Signature; }
        bool IsComponentRemovable() const { return m_IsRemovable; }
    protected:
        ComponentSignature m_Signature{"Default"};
        bool m_IsRemovable{false};
        Scene& m_Scene;
        Registry& m_Registry;
    };

#define COMPONENT_UI_DESC_SIGNATURE(x) static constexpr Engine::ComponentUIDescBase::ComponentSignature GetStaticSignature() { return #x; }
    
    template <typename T>
    class ComponentUIDesc : public ComponentUIDescBase
    {
    public:
        ComponentUIDesc(const ComponentSignature& signature, bool isRemovable, Scene& scene);
        void OnUIDraw(Entity e) override;
        U64 GetComponentID() const override;
        void RemoveComponent(Entity e) override;
        bool ShouldDraw(Entity e) override;
    protected:
        virtual void OnUIDraw([[maybe_unused]] Entity e, T& component) = 0;

    private:
        U64 m_ComponentId{};
    };

    template <typename T>
    ComponentUIDesc<T>::ComponentUIDesc(const ComponentSignature& signature, bool isRemovable, Scene& scene)
        : ComponentUIDescBase(signature, isRemovable, scene),
          m_ComponentId(ComponentFamily::TYPE<T>)
    {
    }

    template <typename T>
    void ComponentUIDesc<T>::OnUIDraw(Entity e)
    {
        if (m_Registry.Has<T>(e)) OnUIDraw(e, m_Registry.Get<T>(e));
    }

    template <typename T>
    U64 ComponentUIDesc<T>::GetComponentID() const
    {
        return m_ComponentId;
    }

    template <typename T>
    void ComponentUIDesc<T>::RemoveComponent(Entity e)
    {
        m_Registry.Remove<T>(e);
    }

    template <typename T>
    bool ComponentUIDesc<T>::ShouldDraw(Entity e)
    {
        return m_Registry.Has<T>(e);
    }

    class LocalToWorldTransformUIDesc : public ComponentUIDesc<Component::LocalToWorldTransform2D>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(Transform2D)
        LocalToWorldTransformUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::LocalToWorldTransform2D& component) override;
        bool ShouldDraw(Entity e) override;
    };

    class LocalToParentTransformUIDesc : public ComponentUIDesc<Component::LocalToParentTransform2D>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(Transform2D)
        LocalToParentTransformUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::LocalToParentTransform2D& component) override;
    };

    class NameUIDesc : public ComponentUIDesc<Component::Name>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(Name)
        NameUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::Name& component) override;
    };

    class BoxCollider2DUIDesc : public ComponentUIDesc<Component::BoxCollider2D>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(BoxCollider2D)
        BoxCollider2DUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::BoxCollider2D& component) override;
    };

    class RigidBody2DUIDesc : public ComponentUIDesc<Component::RigidBody2D>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(RigidBody2D) 
        RigidBody2DUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::RigidBody2D& component) override;
    };

    class SpriteRendererUIDesc : public ComponentUIDesc<Component::SpriteRenderer>
    {
    public:
        COMPONENT_UI_DESC_SIGNATURE(SpriteRenderer) 
        SpriteRendererUIDesc(Scene& scene);
        void OnUIDraw(Entity e, Component::SpriteRenderer& component) override;
    };
    
}
