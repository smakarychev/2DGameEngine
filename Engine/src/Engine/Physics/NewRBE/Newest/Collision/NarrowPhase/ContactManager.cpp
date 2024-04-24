#include "enginepch.h"

#include "ContactManager.h"

#include "PolygonContact.h"

namespace Engine::WIP::Physics::Newest
{
    ContactRegistration ContactManager::s_Registry
            [static_cast<U32>(Collider2DType::TypesCount)]
            [static_cast<U32>(Collider2DType::TypesCount)];
    bool ContactManager::s_IsInit{false};
    
    // Create matrix of dispatch functions.
    void ContactManager::Init()
    {
        AddRegistration(PolygonPolygonContact2D::Create, Collider2DType::Polygon, Collider2DType::Polygon);
        //AddRegistration(CircleCircleContact2D::Create, Collider2DType::Circle, Collider2DType::Circle);
        //AddRegistration(EdgeCircleContact2D::Create, Collider2DType::Edge, Collider2DType::Circle);
    }

    void ContactManager::AddRegistration(ContactRegistration::OnCreateFn createFn, Collider2DType typeA,
        Collider2DType typeB)
    {
        U32 typeAI = static_cast<U32>(typeA);
        U32 typeBI = static_cast<U32>(typeB);
        s_Registry[typeAI][typeBI].CreateFn = createFn;
        s_Registry[typeAI][typeBI].IsPrimary = true;

        if (typeAI != typeBI)
        {
            s_Registry[typeBI][typeAI].CreateFn = createFn;
            s_Registry[typeBI][typeAI].IsPrimary = false;
        }
    }

    Contact2D* ContactManager::Create(ContactAllocator& alloc, Collider2D* a, Collider2D* b)
    {
        if (!s_IsInit)
        {
            Init();
            s_IsInit = true;
        }

        U32 aTypeInt = a->GetTypeInt();
        U32 bTypeInt = b->GetTypeInt();
        if (s_Registry[aTypeInt][bTypeInt].IsPrimary) return s_Registry[aTypeInt][bTypeInt].CreateFn(alloc, a, b);

        return s_Registry[aTypeInt][bTypeInt].CreateFn(alloc, b, a);
    }
}
