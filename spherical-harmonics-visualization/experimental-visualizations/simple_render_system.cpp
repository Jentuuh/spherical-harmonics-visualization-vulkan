#include "simple_render_system.hpp"

// std
#include <cassert>
#include <stdexcept>
#include <array>
#include <chrono>
#include <math.h>


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include<glm/gtc/constants.hpp>

namespace vae {

	SimpleRenderSystem::SimpleRenderSystem(VvtDevice &device, VkRenderPass sceneRenderPass,  VkRenderPass skyboxRenderPass, VkDescriptorSetLayout globalSetLayout) : vvtDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(sceneRenderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(vvtDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(TestPushConstant);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vvtDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw new std::runtime_error("Failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Pipeline layout should be created before pipeline creation!");

		PipelineConfigInfo pipelineConfig{};
		VvtPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vvtPipeline = std::make_unique<VvtPipeline>(vvtDevice, "../Shaders/simple_shader.vert.spv", "../Shaders/simple_shader.frag.spv", pipelineConfig);
	}

	// TODO: State update of objects should be handled somewhere else!
	// Render loop
	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, VkDescriptorSet globalDescriptorSet, std::vector<VvtGameObject> &gameObjects, std::vector<SphereContainer>& sphereFunctions, const VvtCamera& camera, const float frameDeltaTime, VvtGameObject* viewerObj)
	{
		// ===========
		// Draw scene
		// ===========
		vvtPipeline->bind(commandBuffer);
		// Global descriptor set (index 0), can be reused by all game objects
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&globalDescriptorSet, 0,
			nullptr);

		glm::vec3 nextPosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 nextRotation{ 0.0f, 0.0f, 0.0f };


		// Draw gameobjects
		for (auto& obj : gameObjects) {

			TestPushConstant push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
			push.color = obj.color;

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(TestPushConstant),
				&push);

			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);

			// Draw children
			for (auto& child : obj.getChildren()) {

				TestPushConstant pushChild{};
				pushChild.modelMatrix = child.transform.mat4();
				pushChild.normalMatrix = child.transform.normalMatrix();
				pushChild.color = obj.color;

				vkCmdPushConstants(
					commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(TestPushConstant),
					&pushChild);

				child.model->bind(commandBuffer);
				child.model->draw(commandBuffer);
			}
		}

		for (auto& sph : sphereFunctions) {
			sph.render(commandBuffer, pipelineLayout);
		}
	}
}