#pragma once
#include "vvt_swap_chain.hpp"
#include "vvt_pipeline.hpp"
#include "vvt_device.hpp"
#include "vvt_window.hpp"
#include "vvt_game_object.hpp"

// std 
#include <memory>
#include <cassert>

namespace vvt {
	class VvtRenderer
	{
	public:

		VvtRenderer(VvtWindow& window, VvtDevice& device);
		~VvtRenderer();

		VvtRenderer(const VvtRenderer&) = delete;
		VvtRenderer& operator=(const VvtRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vvtSwapChain->getRenderPass(); };
		VkRenderPass getImGuiRenderPass() const { return vvtSwapChain->getImGuiRenderPass(); };
		float getAspectRatio() const { return vvtSwapChain->extentAspectRatio(); };
		bool isFrameInProgress() const { return isFrameStarted; };
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot access command buffer when frame not in progress!");
			return commandBuffers[currentFrameIndex];
		};
		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot access frame index when frame not in progress!");
			return currentFrameIndex;
		};

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void beginImGuiRenderPass(VkCommandBuffer commandBuffer);
		void endImGuiRenderPass(VkCommandBuffer commandBuffer);


	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();


		VvtWindow& vvtWindow;
		VvtDevice& vvtDevice;
		std::unique_ptr<VvtSwapChain> vvtSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted;
	};
}

