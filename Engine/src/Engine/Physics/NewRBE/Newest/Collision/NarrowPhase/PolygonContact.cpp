#include "enginepch.h"

#include "PolygonContact.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine::WIP::Physics::Newest
{
    PolygonPolygonContact2D::PolygonPolygonContact2D(PolygonCollider2D* first, PolygonCollider2D* second)
        : m_First(first), m_Second(second)
    {
    }

    U32 PolygonPolygonContact2D::GenerateContacts(ContactInfo2D& info)
    {
        // Helper functions.
        static auto getVertexTf = [](PolygonCollider2D* col, U32 index)
        {
            return col->GetRigidBody()->GetTransform().Transform(col->GetVertices()[index]);
        };
        static auto getNormalTf = [](PolygonCollider2D* col, U32 index)
        {
            return col->GetRigidBody()->GetTransform().TransformDirection(col->GetNormals()[index]);
        };
        
        ContactManifold2D& manifold = info.Manifold;
        F32 radius = m_First->GetRadius() + m_Second->GetRadius();
        
        // SAT.
        SATQuery satA = SATFaceDirections(*m_First, *m_Second,
                                          m_First->GetRigidBody()->GetTransform(), m_Second->GetRigidBody()->GetTransform());
        if (satA.Distance > radius) return 0;

        SATQuery satB = SATFaceDirections(*m_Second, *m_First,
                                          m_Second->GetRigidBody()->GetTransform(), m_First->GetRigidBody()->GetTransform());
        if (satB.Distance > radius) return 0;

        // We need the axis with the smallest depth.
        SATQuery sat;
        PolygonCollider2D* primary;
        PolygonCollider2D* secondary;
        // Add some bias to satA for coherency.
        static constexpr auto satBias = 0.005f;
        if (satA.Distance > satB.Distance - satBias)
        {
            sat = satA;
            primary = m_First;
            secondary = m_Second;
        }
        else
        {
            sat = satB;
            primary = m_Second;
            secondary = m_First;
        }
        glm::vec2 refFaceDir = Math::Cross2D(getNormalTf(primary, sat.FaceIndex), 1.0f);
        I32 incidentFaceI = FindIncidentFaceIndex(*secondary,
                                                  refFaceDir,
                                                  secondary->GetRigidBody()->GetTransform());

        // Clip incident face by side planes of reference face.
        // Note that ref face in a normal / anti-normal of side planes.
        LineSegment2D incidentFace{
            .Start = getVertexTf(secondary, incidentFaceI),
            .End = getVertexTf(secondary, incidentFaceI < static_cast<I32>(secondary->GetVertices().size()) - 1
                                              ? incidentFaceI + 1
                                              : 0)
        };
        Line2D sideA{
            .Offset = -glm::dot(getVertexTf(primary, sat.FaceIndex), refFaceDir) + radius,
            .Normal = refFaceDir
        };
        Line2D sideB{
            .Offset = glm::dot(getVertexTf(primary, sat.FaceIndex < static_cast<I32>(primary->GetVertices().size()) - 1
                                                        ? sat.FaceIndex + 1
                                                        : 0), refFaceDir) + radius,
            .Normal = -refFaceDir
        };

        ClipLineSegmentToLine(incidentFace, incidentFace, sideA);
        ClipLineSegmentToLine(incidentFace, incidentFace, sideB);

        // Keep all points below reference face.
        std::array<glm::vec2, 2> clippedPoints{incidentFace.Start, incidentFace.End};
        glm::vec2 refFaceNormal = glm::vec2{-refFaceDir.y, refFaceDir.x};
        F32 refFaceOffset = -glm::dot(refFaceNormal, getVertexTf(primary, sat.FaceIndex));

        info.ContactPair.First = primary;
        info.ContactPair.Second = secondary;
        manifold.LocalNormal = primary->GetRigidBody()->GetTransform().InverseTransformDirection(-refFaceNormal);
        manifold.ContactCount = 0;
        manifold.LocalReferencePoint =  0.5f * (
            getVertexTf(primary, sat.FaceIndex) +
            getVertexTf(primary, sat.FaceIndex < static_cast<I32>(primary->GetVertices().size()) - 1
                                                                     ? sat.FaceIndex + 1
                                                                     : 0));
        manifold.LocalReferencePoint = primary->GetRigidBody()->GetTransform().InverseTransform(manifold.LocalReferencePoint);

        for (U32 i = 0; i < clippedPoints.size(); i++)
        {
            // Compute distance to reference face.
            F32 distance = glm::dot(clippedPoints[i], refFaceNormal) + refFaceOffset;
            if (distance > radius) continue;

            ContactPoint2D contactInfo{
                .LocalPoint = secondary->GetRigidBody()->GetTransform().InverseTransform(clippedPoints[i]),
                .PenetrationDepth = -distance
            };
            manifold.Contacts[manifold.ContactCount] = contactInfo;
            manifold.ContactCount++;
        }

        return manifold.ContactCount;
    }

    Contact2D* PolygonPolygonContact2D::Create(ContactAllocator& alloc, Collider2D* a, Collider2D* b)
    {
        return NewAlloc<PolygonPolygonContact2D>(
            alloc,
            static_cast<PolygonCollider2D*>(a),
            static_cast<PolygonCollider2D*>(b)
        );
    }
}
