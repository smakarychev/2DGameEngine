#pragma once

#include "Contact.h"
#include "../Colliders/PolygonCollider2D.h"

namespace Engine::WIP::Physics::Newest
{
    class PolygonPolygonContact2D : public Contact2D
    {
        friend class ContactManager;
    public:
        PolygonPolygonContact2D(PolygonCollider2D* first, PolygonCollider2D* second);

        U32 GenerateContacts(ContactInfo2D& info) override;
        std::array<Collider2D*, 2> GetColliders() override { return { m_First, m_Second }; }
    private:
        static Contact2D* Create(ContactAllocator& alloc, Collider2D* a, Collider2D* b);
    private:
        PolygonCollider2D* m_First, * m_Second;
    };
}
