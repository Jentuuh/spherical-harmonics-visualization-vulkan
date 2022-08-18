#pragma once
#include "vvt_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std 
#include <memory>

namespace vvt {

 // TODO: Make `Component` a base type?
 // TODO: Find cleaner solution for chunk management! (each gameobject currently contains a chunk)

    struct TransformComponent {
        glm::vec3 translation{};  // Position offset
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};
        glm::vec3 relativePos{.0f, .0f, .0f};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
     
    class VvtGameObject {

    public:
        using id_t = unsigned int;

        static VvtGameObject createGameObject() {
            static id_t currentId = 0;
            return VvtGameObject{ currentId++ };
        }
        static VvtGameObject createGameObject(id_t id) {
            return VvtGameObject{ id };
        }
        ~VvtGameObject() 
        {
            static id_t currentId = currentId--;
        }

        VvtGameObject(const VvtGameObject&) = delete;
        VvtGameObject& operator=(const VvtGameObject&) = delete;
        VvtGameObject(VvtGameObject&&) = default;
        VvtGameObject& operator=(VvtGameObject&&) = default;

        id_t getId() { return id; }
        std::vector<VvtGameObject>& getChildren() { return children; };

        void addChild(VvtGameObject* child);
        void setPosition(glm::vec3 newPosition);
        void setRotation(glm::vec3 newRotation);
        void setScale(glm::vec3 newScale);

        std::string modelPath;
        std::shared_ptr<VvtModel> model{};
        std::vector<VvtGameObject> children{};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        TransformComponent transform{};

    private:
        glm::vec3 prevPos{0.0f, 0.0f, 0.0f};
        VvtGameObject(id_t objId) : id{ objId } {}
        id_t id;
    };
}

