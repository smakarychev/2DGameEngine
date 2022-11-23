#pragma once
#include "EntityId.h"
#include "Registry.h"
#include "Engine/Core/Types.h"

namespace Engine
{
    using namespace Types;
    
    // Cmpts for components.
    template <typename ... Cmpts>
    class View
    {
    public:
        struct Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = I32;
            using value_type = Entity;
            using pointer = value_type*;
            using reference = value_type&;

            Iterator(const Registry& registry, const std::array<U64, sizeof ...(Cmpts)>& compIds,
                     const ComponentPool* refPool,
                     I32 entityIndex, bool allEntities)
                : Registry(registry), ComponentIds(compIds), ReferencePool(refPool),
                  ReferencePoolCurrentEntityIndex(entityIndex), AllEntities(allEntities)
            {
            }

            value_type operator*() const
            {
                return ReferencePool->GetDenseEntities()[ReferencePoolCurrentEntityIndex];
            }

            bool operator==(const Iterator& other) const
            {
                return ReferencePoolCurrentEntityIndex == other.ReferencePoolCurrentEntityIndex;
            }

            bool operator!=(const Iterator& other) const
            {
                return ReferencePoolCurrentEntityIndex != other.ReferencePoolCurrentEntityIndex;
            }

            Iterator& operator++()
            {
                ReferencePoolCurrentEntityIndex--;
                if (!AllEntities)
                {
                    while (ReferencePoolCurrentEntityIndex >= 0 &&
                        !IsInAllComponents(ReferencePool->GetDenseEntities()[ReferencePoolCurrentEntityIndex]))
                    {
                        ReferencePoolCurrentEntityIndex--;
                    }
                }
                return *this;
            }

        private:
            bool IsInAllComponents(Entity entityId)
            {
                for (auto& cId : ComponentIds)
                {
                    if (!Registry.GetComponentPool(cId).Has(entityId))
                    {
                        return false;
                    }
                }
                return true;
            }

        public:
            const Registry& Registry;
            std::array<U64, sizeof ...(Cmpts)> ComponentIds;
            // Reference pool is the one we take EntityId objects from.
            const ComponentPool* ReferencePool = nullptr;
            I32 ReferencePoolCurrentEntityIndex;
            bool AllEntities = false;
        };

    public:
        View(const Registry& registry)
            : m_Registry(registry)
        {
            if (sizeof ...(Cmpts) == 0)
            {
                m_AllEntities = true;
                // If we wish to return all entities, the simplest way is
                // to iterate over entities of component pool, that has every entity,
                // in our case it is either "tag" component or "transform2d" component,
                // however when moving outside of 2d-only, only "tag" component will do.
                m_ReferencePool = &m_Registry.GetComponentPool<Component::Tag>();
            }
            else
            {
                m_ComponentIds = {{ComponentFamily::TYPE<Cmpts>...}};
                // Check that registry actually has all components.
                for (auto id : m_ComponentIds)
                {
                    ENGINE_ASSERT(m_Registry.IsComponentExists(id), "Registry does not have that component")
                }
                // Sort the components based on how many entities are in them,
                // for faster search.
                std::sort(m_ComponentIds.begin(), m_ComponentIds.end(), [this](auto& a, auto& b)
                {
                    return m_Registry.GetComponentPool(a).GetComponentCount() <
                        m_Registry.GetComponentPool(b).GetComponentCount();
                });
                m_ReferencePool = &m_Registry.GetComponentPool(m_ComponentIds.front());
            }
        }

        Iterator begin() const
        {
            return Iterator(m_Registry, m_ComponentIds, m_ReferencePool, static_cast<U32>(m_ReferencePool->GetDenseEntities().size()) - 1, m_AllEntities);
        }

        Iterator end() const
        {
            return Iterator(m_Registry, m_ComponentIds, m_ReferencePool,
                            -1, m_AllEntities);
        }

    private:
        const Registry& m_Registry;
        std::array<U64, sizeof ...(Cmpts)> m_ComponentIds;
        // Reference pool is the one we take EntityId objects from.
        const ComponentPool* m_ReferencePool = nullptr;
        bool m_AllEntities = false;
    };
}
