#pragma once
#include "vvt_model.hpp"
#include "vvt_game_object.hpp"
#include "basis_container.hpp"

#include <spherical_harmonics.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define MONTE_CARLO_SAMPLE_AMOUNT 10000
#define BASIS_FUNCTION_MAX_ORDER 4

namespace vvt {

	class SphereContainer {
	public:
		SphereContainer(glm::vec3 pos, glm::vec3 rot, float radius, sh::SphericalFunction sphFunc, std::shared_ptr<VvtModel> model);

		glm::vec3& getRotation() { return transform.rotation; };

		void generateSpherePoints();
		void updateRotation();
		void render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

	private:
		void addSphere3DPoint(double phi, double theta);
		void addSphere3DPointReconstructed(double phi, double theta);
		void decomposeToBasisFunctions(int order, int samples);
		void visualizeBasisFunctions();
		void generateReconstruction();
		
		float radius;
		sh::SphericalFunction sphFunc;
		std::shared_ptr<VvtModel> pointModel;
		TransformComponent transform;
		std::vector<std::pair<glm::vec3, double>> points;		
		std::vector<std::pair<glm::vec3, double>> pointsReconstructed;

		std::vector<glm::vec3> ogPointPositions;
		std::vector<double> basisCoeffs;
		std::vector<BasisContainer> basisFunctions;

		int resolution = 100;
	};
}