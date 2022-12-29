#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Camera.h"
#include "yaml-cpp/yaml.h"

namespace YAML
{
    template <>
    struct convert<Engine::Entity>
    {
        static Node encode(Engine::Entity rhs)
        {
            Node node;
            node.push_back(rhs.Id);
            return node;
        }

        static bool decode(const Node& node, Engine::Entity& rhs)
        {
            rhs.Id = node.as<Engine::Types::U32>();
            return true;
        }
    };
    
    template <typename T>
    struct convert<glm::vec<2, T, glm::defaultp>>
    {
        static Node encode(const glm::vec<2, T, glm::defaultp>& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, glm::vec<2, T, glm::defaultp>& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
            {
                return false;
            }

            rhs.x = node[0].as<T>();
            rhs.y = node[1].as<T>();
            return true;
        }
    };

    template <typename T>
    struct convert<glm::vec<3, T, glm::defaultp>>
    {
        static Node encode(const glm::vec<3, T, glm::defaultp>& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec<3, T, glm::defaultp>& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
            {
                return false;
            }

            rhs.x = node[0].as<T>();
            rhs.y = node[1].as<T>();
            rhs.z = node[2].as<T>();
            return true;
        }
    };

    template <typename T>
    struct convert<glm::vec<4, T, glm::defaultp>>
    {
        static Node encode(const glm::vec<4, T, glm::defaultp>& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec<4, T, glm::defaultp>& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }

            rhs.x = node[0].as<T>();
            rhs.y = node[1].as<T>();
            rhs.z = node[2].as<T>();
            rhs.w = node[3].as<T>();
            return true;
        }
    };

    template <typename T>
    Emitter& operator<<(Emitter& out, const glm::vec<2, T, glm::defaultp>& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << EndSeq;
        return out;
    }

    template <typename T>
    Emitter& operator<<(Emitter& out, const glm::vec<3, T, glm::defaultp>& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << vec.z << EndSeq;
        return out;
    }

    template <typename T>
    Emitter& operator<<(Emitter& out, const glm::vec<4, T, glm::defaultp>& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << vec.z << vec.w << EndSeq;
        return out;
    }
}

namespace Engine::SerializationUtils
{
    // TODO: compile-time maps.
    
    inline std::string GetRigidBodyTypeAsString(const Component::RigidBody2D& rb)
    {
        if (rb.Type == Physics::RigidBodyType2D::Static) return "Static";
        return "Dynamic";
    }

    inline Physics::RigidBodyType2D GetRigidBodyTypeFromString(const std::string& type)
    {
        if (type == "Static") return Physics::RigidBodyType2D::Static;
        return Physics::RigidBodyType2D::Dynamic;
    }
    
    inline std::vector<std::string> GetRigidBodyFlagsAsStrings(const Component::RigidBody2D& rb)
    {
        std::vector<std::string> result;
        static std::unordered_map<Physics::RigidBodyDef2D::BodyFlags, std::string> flagMap {
            std::make_pair(Physics::RigidBodyDef2D::BodyFlags::RestrictRotation, "Restrict Rotation"),
            std::make_pair(Physics::RigidBodyDef2D::BodyFlags::UseSyntheticMass, "Use synthetic mass")
        };
        auto flags = rb.Flags;
        for (auto&& [flag, str] : flagMap)
        {
            if (flags & flag) result.push_back(str);
        }
        return result;
    }

    inline Physics::RigidBodyDef2D::BodyFlags GetBodyFlagsFromStrings(const std::vector<std::string>& flags)
    {
        Physics::RigidBodyDef2D::BodyFlags result = Physics::RigidBodyDef2D::BodyFlags::None;
        static std::unordered_map<std::string, Physics::RigidBodyDef2D::BodyFlags> flagMap {
            std::make_pair("Restrict Rotation", Physics::RigidBodyDef2D::BodyFlags::RestrictRotation),
            std::make_pair("Use synthetic mass", Physics::RigidBodyDef2D::BodyFlags::UseSyntheticMass)
        };
        for (auto& flag : flags)
        {
            auto it = flagMap.find(flag);
            if (it == flagMap.end()) ENGINE_CORE_ERROR("Unexpected flag: {}", flag);
            result = static_cast<Physics::RigidBodyDef2D::BodyFlags>(result | it->second);
        }
        return result;
    }

    struct CameraControllerHasher
    {
        template <typename T>
        std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
    };
        
    inline std::string GetCameraControllerTypeAsString(const Component::Camera& camera)
    {
        static std::unordered_map<CameraController::ControllerType, std::string, CameraControllerHasher> typeMap {
            std::make_pair(CameraController::ControllerType::FPS, "FPS"),
            std::make_pair(CameraController::ControllerType::Editor, "Editor"),
            std::make_pair(CameraController::ControllerType::Editor2D, "Editor2D"),
            std::make_pair(CameraController::ControllerType::Custom, "Custom")
        };
        auto it = typeMap.find(camera.CameraController->GetControllerType());
        return it->second;
    }

    inline CameraController::ControllerType GetCameraControllerTypeFromString(const std::string& type)
    {
        static std::unordered_map<std::string, CameraController::ControllerType> typeMap {
            std::make_pair("FPS", CameraController::ControllerType::FPS),
            std::make_pair("Editor", CameraController::ControllerType::Editor),
            std::make_pair("Editor2D", CameraController::ControllerType::Editor2D),
            std::make_pair("Custom", CameraController::ControllerType::Custom)
        };
        auto it = typeMap.find(type);
        return it->second;
    }
    
}
