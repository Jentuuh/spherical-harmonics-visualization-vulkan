#pragma once
#include "vvt_model.hpp"
#include "spherical_harmonics.h"
#include "vvt_game_object.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace vae {
	class BasisContainer
	{
	public:
		BasisContainer(double coeff, int order, int degree, float radius, glm::vec3 pos, glm::vec3 rot);

		void render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VvtModel> pointModel);

	private:
		void generatePoints(int order, int degree);
		void addSphere3DPoint(double phi, double theta);

		double coefficient;
		int order;
		int degree;		
		float radius;
		TransformComponent transform;

		std::vector<std::pair<glm::vec3, double>> points;

		int resolution = 80;
	};
}
