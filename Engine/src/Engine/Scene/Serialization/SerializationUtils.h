#pragma once

#include <glm/glm.hpp>

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
    
    template <>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
            {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
            {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
    
    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    inline Emitter& operator<<(Emitter& out, const glm::vec2& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << EndSeq;
        return out;
    }

    inline Emitter& operator<<(Emitter& out, const glm::vec3& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << vec.z << EndSeq;
        return out;
    }

    inline Emitter& operator<<(Emitter& out, const glm::vec4& vec)
    {
        out << Flow;
        out << BeginSeq << vec.x << vec.y << vec.z << vec.w << EndSeq;
        return out;
    }
}

namespace Engine
{
    inline std::vector<std::string> GetRigidBodyFlagsAsStrings(const Component::RigidBody2D& rb)
    {
        std::vector<std::string> result;
        static std::unordered_map<Physics::RigidBodyDef2D::BodyFlags, std::string> flagMap;
        flagMap.emplace(Physics::RigidBodyDef2D::BodyFlags::RestrictRotation, "Restrict Rotation");
        flagMap.emplace(Physics::RigidBodyDef2D::BodyFlags::UseSyntheticMass, "Use synthetic mass");
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
        static std::unordered_map<std::string, Physics::RigidBodyDef2D::BodyFlags> flagMap;
        flagMap.emplace("Restrict Rotation", Physics::RigidBodyDef2D::BodyFlags::RestrictRotation);
        flagMap.emplace("Use synthetic mass", Physics::RigidBodyDef2D::BodyFlags::UseSyntheticMass);
        for (auto& flag : flags)
        {
            auto it = flagMap.find(flag);
            if (it == flagMap.end()) ENGINE_CORE_ERROR("Unexpected flag: {}", flag);
            result = static_cast<Physics::RigidBodyDef2D::BodyFlags>(result | it->second);
        }
        return result;
    }
}
