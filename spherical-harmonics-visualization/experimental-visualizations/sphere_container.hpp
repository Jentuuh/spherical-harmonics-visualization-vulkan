#pragma once
#include "vvt_model.hpp"
#include "vvt_game_object.hpp"

#include <spherical_harmonics.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace vae {

	class SphereContainer {
	public:
		SphereContainer(glm::vec3 pos, glm::vec3 rot, float radius, sh::SphericalFunction sphFunc, std::shared_ptr<VvtModel> model);

		void generateSpherePoints();
		void render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

	private:
		void addSphere3DPoint(double phi, double theta);

		sh::SphericalFunction sphFunc;
		std::shared_ptr<VvtModel> pointModel;

		TransformComponent transform;
		float radius;
		std::vector<std::pair<glm::vec3, double>> points;

		int resolution = 100;
	};
}