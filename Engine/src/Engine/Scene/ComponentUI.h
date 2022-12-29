#pragma once
#include "Engine/ECS/Components.h"
#include "Engine/ECS/Registry.h"

namespace Engine
{
    class Registry;
}

namespace Engine
{
    class ComponentUIDescBase
    {
    public:
        ComponentUIDescBase(const std::string& name, bool isRemovable, Registry& registry);
        virtual ~ComponentUIDescBase();
        virtual void OnUIDraw(Entity e) = 0;
        virtual U64 GetComponentID() const = 0;
        virtual void RemoveComponent(Entity e) = 0;
        virtual bool ShouldDraw(Entity e) = 0;
        
        Registry& GetAttachedRegistry() const { return m_Registry; }
        const std::string& GetComponentName() const { return m_ComponentName; }
        bool IsComponentRemovable() const { return m_IsRemovable; }
    protected:
        std::string m_ComponentName{"Default"};
        bool m_IsRemovable{false};
        Registry& m_Registry;
    };

    template <typename T>
    class ComponentUIDesc : public ComponentUIDescBase
    {
    public:
        ComponentUIDesc(const std::string& name, bool isRemovable, Registry& registry);
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
    ComponentUIDesc<T>::ComponentUIDesc(const std::string& name, bool isRemovable, Registry& registry)
        : ComponentUIDescBase(name, isRemovable, registry),
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
        LocalToWorldTransformUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::LocalToWorldTransform2D& component) override;
        bool ShouldDraw(Entity e) override;
    };

    class LocalToParentTransformUIDesc : public ComponentUIDesc<Component::LocalToParentTransform2D>
    {
    public:
        LocalToParentTransformUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::LocalToParentTransform2D& component) override;
    };

    class TagUIDesc : public ComponentUIDesc<Component::Tag>
    {
    public:
        TagUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::Tag& component) override;
    };

    class NameUIDesc : public ComponentUIDesc<Component::Name>
    {
    public:
        NameUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::Name& component) override;
    };

    class BoxCollider2DUIDesc : public ComponentUIDesc<Component::BoxCollider2D>
    {
    public:
        BoxCollider2DUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::BoxCollider2D& component) override;
    };

    class RigidBody2DUIDesc : public ComponentUIDesc<Component::RigidBody2D>
    {
    public:
        RigidBody2DUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::RigidBody2D& component) override;
    };

    class SpriteRendererUIDesc : public ComponentUIDesc<Component::SpriteRenderer>
    {
    public:
        SpriteRendererUIDesc(Registry& registry);
        void OnUIDraw(Entity e, Component::SpriteRenderer& component) override;
    };
    
}
