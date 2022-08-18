#include "sphere_container.hpp"
#include "simple_render_system.hpp"
#include <iostream>
namespace vvt {
	SphereContainer::SphereContainer(glm::vec3 pos, glm::vec3 rot, float radius, sh::SphericalFunction sphFunc, std::shared_ptr<VvtModel> model): radius{radius}, 																																		  sphFunc{sphFunc}, pointModel{model}
	{
		transform.translation = pos;
		transform.rotation = rot;
		transform.scale = { 1.0f, 1.0f, 1.0f };
		decomposeToBasisFunctions(BASIS_FUNCTION_MAX_ORDER, MONTE_CARLO_SAMPLE_AMOUNT);
		visualizeBasisFunctions();
		generateReconstruction();
	}

	void SphereContainer::generateSpherePoints()
	{
		for (int i = 0; i < resolution; i++)
		{
			for (int j = 0; j < resolution; j++)
			{
				float phi = (static_cast<float>(i) / static_cast<float>(resolution)) * 2 * glm::pi<float>();
				float theta = (static_cast<float>(j) / static_cast<float>(resolution)) * glm::pi<float>();

				addSphere3DPoint(phi, theta);
			}
		}
	}

	void SphereContainer::updateRotation()
	{
		glm::mat4 rotMatrix = transform.mat4();

		for (int i = 0; i < points.size(); i++)
		{
			// Rotate original sphere function and reconstructed sphere function
			glm::vec4 rotatedPoint = rotMatrix * glm::vec4{ ogPointPositions[i], 1.0f};
			points[i].first = {rotatedPoint.x, rotatedPoint.y, rotatedPoint.z};
			pointsReconstructed[i].first = glm::vec3{ rotatedPoint.x, rotatedPoint.y, rotatedPoint.z } + glm::vec3{ -(2 * radius + 1.0f), 0.0f, 0.0f };
		}
	}


	void SphereContainer::render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout)
	{
		// Draw spherical function
		for (auto& p : points)
		{
			TransformComponent pointTransform;
			pointTransform.translation = p.first;
			pointTransform.scale = { 0.03f, 0.03f, 0.03f };
	
			TestPushConstant push{};
			push.modelMatrix = pointTransform.mat4();
			push.normalMatrix = pointTransform.normalMatrix();
			if (p.second < 0.0f)
			{
				push.color = { 0.0f, 0.0f, 1.0f * abs(p.second) };
			}
			else {
				push.color = { 1.0f * abs(p.second), 0.0f, 0.0f };
			}

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(TestPushConstant),
				&push);

			pointModel->bind(commandBuffer);
			pointModel->draw(commandBuffer);
		}

		// Draw reconstructed spherical function
		for (auto& p : pointsReconstructed)
		{
			TransformComponent pointTransform;
			pointTransform.translation = p.first;
			pointTransform.scale = { 0.03f, 0.03f, 0.03f };

			TestPushConstant push{};
			push.modelMatrix = pointTransform.mat4();
			push.normalMatrix = pointTransform.normalMatrix();
			if (p.second < 0.0f)
			{
				push.color = { 0.0f, 0.0f, 1.0f * abs(p.second) };
			}
			else {
				push.color = { 1.0f * abs(p.second), 0.0f, 0.0f };
			}

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(TestPushConstant),
				&push);

			pointModel->bind(commandBuffer);
			pointModel->draw(commandBuffer);
		}

		// Draw basis functions
		for (auto& b : basisFunctions)
		{
			b.render(commandBuffer, pipelineLayout, pointModel);
		}

	}


	void SphereContainer::addSphere3DPoint(double phi, double theta)
	{
		Eigen::Vector3d dirVectorFromSphericalCoords = sh::ToVector(phi, theta);
		glm::vec3 glmDirVector = { dirVectorFromSphericalCoords.x(), dirVectorFromSphericalCoords.y(), dirVectorFromSphericalCoords.z() };

		glm::vec3 pointPos = transform.translation + glm::normalize(glmDirVector) * radius;
		double pointValue = sphFunc(phi, theta); 

		points.push_back(std::make_pair(pointPos, pointValue));
		ogPointPositions.push_back(pointPos);
	}

	void SphereContainer::addSphere3DPointReconstructed(double phi, double theta)
	{
		Eigen::Vector3d dirVectorFromSphericalCoords = sh::ToVector(phi, theta);
		glm::vec3 glmDirVector = { dirVectorFromSphericalCoords.x(), dirVectorFromSphericalCoords.y(), dirVectorFromSphericalCoords.z() };

		// Translated to the left of the original spherical function
		glm::vec3 pointPos = transform.translation + glm::vec3{-(2 * radius + 1.0f), 0.0f, 0.0f} + glm::normalize(glmDirVector) * radius;
		double pointValue = sh::EvalSHSum(BASIS_FUNCTION_MAX_ORDER, basisCoeffs, phi, theta);

		pointsReconstructed.push_back(std::make_pair(pointPos, pointValue));
	}


	void SphereContainer::decomposeToBasisFunctions(int order, int samples)
	{
		std::unique_ptr<std::vector<double>> coeffs = sh::ProjectFunction(order, sphFunc, samples);
		basisCoeffs = *coeffs;
	}

	void SphereContainer::visualizeBasisFunctions()
	{
		int index = 0;
		for (int i = 0; i <= BASIS_FUNCTION_MAX_ORDER; i++)
		{
			for (int j = 0; j <= i; j++)
			{
				if (j == 0) {
					BasisContainer basis0 = {basisCoeffs[index], i, 0, radius, transform.translation + glm::vec3{(2 * radius + 1.0f) * (i + 1), 0.0f, 0.0f}, transform.rotation };
					basisFunctions.push_back(basis0);
					index++;
				}
				else {
					BasisContainer basis_j_pos = { basisCoeffs[index], i, -j, radius, transform.translation + glm::vec3{(2 * radius + 1.0f)* (i + 1), j * -(2 * radius + 1.0f), 0.0f}, transform.rotation };
					basisFunctions.push_back(basis_j_pos);
					index++;

					BasisContainer basis_j_neg = { basisCoeffs[index], i, j, radius, transform.translation + glm::vec3{(2 * radius + 1.0f)* (i + 1), j * (2 * radius + 1.0f), 0.0f}, transform.rotation };
					basisFunctions.push_back(basis_j_neg);
					index++;
				}
			}
		}
	}

	void SphereContainer::generateReconstruction()
	{
		for (int i = 0; i < resolution; i++)
		{
			for (int j = 0; j < resolution; j++)
			{
				float phi = (static_cast<float>(i) / static_cast<float>(resolution)) * 2 * glm::pi<float>();
				float theta = (static_cast<float>(j) / static_cast<float>(resolution)) * glm::pi<float>();

				addSphere3DPointReconstructed(phi, theta);
			}
		}
	}
}