#pragma once

#include "vvt_camera.hpp"
#include "vvt_pipeline.hpp"
#include "vvt_device.hpp"
#include "vvt_game_object.hpp"
#include "sphere_container.hpp"

// std 
#include <memory>
#include <vector>

namespace vvt {
	struct TestPushConstant {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
		glm::vec3 color{ 1.f };
	};

	class SimpleRenderSystem
	{
	public:

		SimpleRenderSystem(VvtDevice& device, VkRenderPass sceneRenderPass, VkRenderPass skyboxRenderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, VkDescriptorSet globalDescriptorSet,
								std::vector<VvtGameObject> &gameObjects, std::vector<SphereContainer> &sphereFunctions, 
								const VvtCamera& camera, const float frameDeltaTime, VvtGameObject* viewerObj);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VvtDevice& vvtDevice;

		float clock;
		std::unique_ptr<VvtPipeline> vvtPipeline;
		VkPipelineLayout pipelineLayout;
	};
}
