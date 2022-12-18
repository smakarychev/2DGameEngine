#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

namespace Engine
{
    using namespace Types;

    class RefCountBase
    {
    public:
        virtual ~RefCountBase() {}
        void Increment();
        void Decrement();
        U32 Get() { return m_Count; }
        virtual void Release() {}
        virtual void Destroy() {}
    protected:
        U32 m_Count{};
    };

    inline void RefCountBase::Increment()
    {
        m_Count++;
    }

    inline void RefCountBase::Decrement()
    {
        ENGINE_CORE_ASSERT(m_Count != 0, "Can not decrement")
        m_Count--;
        if (m_Count == 0)
        {
            Release();
            Destroy();
        }
    }

    template <typename T>
    class RefCount : public RefCountBase
    {
    public:
        RefCount(T* object);
        void Release() override;
        void Destroy() override;
    private:
        T* m_Object{};
    };

    template <typename T>
    RefCount<T>::RefCount(T* object)
        : m_Object(object)
    {
    }

    template <typename T>
    void RefCount<T>::Release()
    {
        Engine::Delete(m_Object);
    }

    template <typename T>
    void RefCount<T>::Destroy()
    {
        Engine::Delete(this);
    }

    template <typename T, typename Del>
    class RefCountCustomDel : public RefCountBase
    {
    public:
        RefCountCustomDel(T* object, Del deleter);
        void Release() override;
        void Destroy() override;
    private:
        T* m_Object{};
        Del m_Deleter;
    };

    template <typename T, typename Del>
    RefCountCustomDel<T, Del>::RefCountCustomDel(T* object, Del deleter)
        : m_Object(object), m_Deleter(deleter)
    {
    }

    template <typename T, typename Del>
    void RefCountCustomDel<T, Del>::Release()
    {
        m_Deleter(m_Object);
    }

    template <typename T, typename Del>
    void RefCountCustomDel<T, Del>::Destroy()
    {
        Engine::Delete(this);
    }

    template <typename T>
    class RefCountHandle
    {
    public:
        RefCountHandle() = default;
        RefCountHandle(nullptr_t) {}
        RefCountHandle(T* object);
        template <typename Del>
        RefCountHandle(T* object, Del deleter);
        RefCountHandle(const RefCountHandle& other);
        RefCountHandle(RefCountHandle&& other) noexcept;
        RefCountHandle& operator=(const RefCountHandle& other);
        RefCountHandle& operator=(RefCountHandle&& other) noexcept;
        ~RefCountHandle();
        T* Get();
        T* Get() const;

        bool operator==(T* ptr);
        bool operator!=(T* ptr);

        T* operator->();
        T* operator->() const;
    private:
        void Swap(RefCountHandle& other);
    private:
        T* m_Object{};
        RefCountBase* m_RefCount{};
    };

    template <typename T>
    RefCountHandle<T>::RefCountHandle(T* object)
        : m_Object(object)
    {
        m_RefCount = New<RefCount<T>>(object);
        m_RefCount->Increment();
    }

    template <typename T>
    template <typename Del>
    RefCountHandle<T>::RefCountHandle(T* object, Del deleter)
        : m_Object(object)
    {
        m_RefCount = New<RefCountCustomDel<T, Del>>(object, deleter);
        m_RefCount->Increment();
    }

    template <typename T>
    RefCountHandle<T>::RefCountHandle(const RefCountHandle& other)
        :  m_Object(other.m_Object), m_RefCount(other.m_RefCount)
    {
        other.m_RefCount->Increment();
    }

    template <typename T>
    RefCountHandle<T>::RefCountHandle(RefCountHandle&& other) noexcept
        : m_Object(other.m_Object), m_RefCount(other.m_RefCount)
    {
        other.m_Object = nullptr;
        other.m_RefCount = nullptr;
    }

    template <typename T>
    RefCountHandle<T>& RefCountHandle<T>::operator=(const RefCountHandle& other)
    {
        RefCountHandle(other).Swap(*this);
        return *this;
    }

    template <typename T>
    RefCountHandle<T>& RefCountHandle<T>::operator=(RefCountHandle&& other) noexcept
    {
        RefCountHandle(std::move(other)).Swap(*this);
        return *this;
    }

    template <typename T>
    RefCountHandle<T>::~RefCountHandle()
    {
        if (m_RefCount) m_RefCount->Decrement();
    }

    template <typename T>
    T* RefCountHandle<T>::Get()
    {
        return m_Object;
    }

    template <typename T>
    T* RefCountHandle<T>::Get() const
    {
        return m_Object;
    }

    template <typename T>
    T* RefCountHandle<T>::operator->() const
    {
        return Get();
    }

    template <typename T>
    bool RefCountHandle<T>::operator==(T* ptr)
    {
        return Get() == ptr;
    }

    template <typename T>
    bool RefCountHandle<T>::operator!=(T* ptr)
    {
        return !(*this == ptr);
    }

    template <typename T>
    T* RefCountHandle<T>::operator->()
    {
        return Get();
    }

    template <typename T>
    void RefCountHandle<T>::Swap(RefCountHandle& other)
    {
        std::swap(m_Object, other.m_Object);
        std::swap(m_RefCount, other.m_RefCount);
    }
}
