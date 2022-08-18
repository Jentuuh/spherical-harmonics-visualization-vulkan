#include "vvt_game_object.hpp"
#include <iostream>

namespace vvt {

    /**
    Constructs and returns a TRS-matrix that corresponds to translation * rotation.y * rotation.x * rotation.z * scale.
    This "moves" the game object from its local object space into world space (model transform).
    https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix (optimized implementation)
    */
    glm::mat4 TransformComponent::mat4() 
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f} };
    }

    /**
    Constructs a normalMatrix to be used in the vertex shader to transform the normals to world space
    (we perform this here to ALLOW FOR NON-UNIFORM SCALING)
    */
    glm::mat3 TransformComponent::normalMatrix()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 inverseScale = 1.f / scale;

        return glm::mat3{
            {
                inverseScale.x * (c1 * c3 + s1 * s2 * s3),
                inverseScale.x * (c2 * s3),
                inverseScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                inverseScale.y * (c2 * c3),
                inverseScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                inverseScale.z * (c2 * s1),
                inverseScale.z * (-s2),
                inverseScale.z * (c1 * c2),
            }, 
        };
    }

    void VvtGameObject::setPosition(glm::vec3 newPosition)
    {
        glm::vec3 transVec = newPosition - prevPos;
        transform.translation = newPosition;
        prevPos = newPosition;

        for (auto& c : children)
        {
            c.setPosition(newPosition);
        }
    }

    void VvtGameObject::setRotation(glm::vec3 newRotation)
    {
        transform.rotation = newRotation;
       
        for (auto& c : children)
        {
            c.setRotation(newRotation);
        }
    }

    void VvtGameObject::setScale(glm::vec3 newScale)
    {
        transform.scale = newScale;

        for (auto& c : children)
        {
            c.setScale(newScale);
        }
    }

    void VvtGameObject::addChild(VvtGameObject* child)
    {
        children.push_back(std::move(*child));
    }
}