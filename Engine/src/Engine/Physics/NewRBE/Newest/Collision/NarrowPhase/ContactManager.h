#pragma once

#include "Contact.h"

namespace Engine::WIP::Physics::Newest
{
    struct ContactRegistration
    {
        using OnCreateFn = Contact2D* (*)(ContactAllocator& alloc, Collider2D*, Collider2D*);
        OnCreateFn CreateFn;
        // So we need twice as less methods.
        bool IsPrimary;
    };

    class ContactManager
    {
    public:
        static void Init();
        static void AddRegistration(
            ContactRegistration::OnCreateFn createFn,
            Collider2DType typeA,
            Collider2DType typeB
        );
        static Contact2D* Create(ContactAllocator& alloc, Collider2D* a, Collider2D* b);

        static ContactRegistration s_Registry
            [static_cast<U32>(Collider2DType::TypesCount)]
            [static_cast<U32>(Collider2DType::TypesCount)];
    private:
        static bool s_IsInit;
    };
}
