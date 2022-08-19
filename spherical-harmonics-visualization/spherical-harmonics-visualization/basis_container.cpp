#include "basis_container.hpp"
#include "simple_render_system.hpp"

namespace vvt {
	BasisContainer::BasisContainer(double coeff, int order, int degree, float radius, glm::vec3 pos, glm::vec3 rot): order{order}, degree{degree}, radius{radius}, coefficient{coeff}
	{
		transform.translation = pos;
		transform.rotation = rot;
		transform.scale = { 1.0f, 1.0f, 1.0f };
		generatePoints(order, degree);
	}

	void BasisContainer::render(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VvtModel> pointModel, double maxCoeff)
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
				push.color =  {0.0f, 0.0f, (abs(coefficient)/maxCoeff) * abs(p.second) };
			}
			else {
				push.color = { (abs(coefficient)/maxCoeff) * abs(p.second), 0.0f, 0.0f };
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


	void BasisContainer::generatePoints(int order, int degree)
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

	void BasisContainer::addSphere3DPoint(double phi, double theta)
	{
		Eigen::Vector3d dirVectorFromSphericalCoords = sh::ToVector(phi, theta);
		glm::vec3 glmDirVector = { dirVectorFromSphericalCoords.x(), dirVectorFromSphericalCoords.y(), dirVectorFromSphericalCoords.z() };

		glm::vec3 pointPos = transform.translation + glm::normalize(glmDirVector) * radius;
		double pointValue = sh::EvalSH(order, degree, phi, theta);

		points.push_back(std::make_pair(pointPos, pointValue));
	}


}