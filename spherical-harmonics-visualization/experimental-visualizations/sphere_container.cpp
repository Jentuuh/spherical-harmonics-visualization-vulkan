#include "sphere_container.hpp"
#include "simple_render_system.hpp"
#include <iostream>
namespace vae {
	SphereContainer::SphereContainer(glm::vec3 pos, glm::vec3 rot, float radius, sh::SphericalFunction sphFunc, std::shared_ptr<VvtModel> model): radius{radius}, 																																		  sphFunc{sphFunc}, pointModel{model}
	{
		transform.translation = pos;
		transform.rotation = rot;
		transform.scale = { 1.0f, 1.0f, 1.0f };
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

	void SphereContainer::render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout)
	{
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

	}


	void SphereContainer::addSphere3DPoint(double phi, double theta)
	{
		Eigen::Vector3d dirVectorFromSphericalCoords = sh::ToVector(phi, theta);
		glm::vec3 glmDirVector = { dirVectorFromSphericalCoords.x(), dirVectorFromSphericalCoords.y(), dirVectorFromSphericalCoords.z() };

		glm::vec3 pointPos = transform.translation + glm::normalize(glmDirVector) * radius;
		double pointValue = sphFunc(phi, theta); 

		points.push_back(std::make_pair(pointPos, pointValue));
	}


}