#include "vvt_renderer.hpp"

// std
#include <cassert>
#include <stdexcept>
#include <array>

namespace vvt {

	VvtRenderer::VvtRenderer(VvtWindow & window, VvtDevice & device) : vvtWindow{window}, vvtDevice{device}
	{
		recreateSwapchain();
		createCommandBuffers();
	}

	VvtRenderer::~VvtRenderer()
	{
		freeCommandBuffers();
	}


	void VvtRenderer::recreateSwapchain()
	{
		auto extent = vvtWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			// Let the program pause and wait when at least 1 dimension is 0.
			extent = vvtWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(vvtDevice.device());
		if (vvtSwapChain == nullptr) {
			vvtSwapChain = std::make_unique<VvtSwapChain>(vvtDevice, extent);
		}
		else {
			std::shared_ptr<VvtSwapChain> oldSwapChain = std::move(vvtSwapChain);
			vvtSwapChain = std::make_unique<VvtSwapChain>(vvtDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*vvtSwapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth image) format has changed!");
			}
		}
	}

	void VvtRenderer::createCommandBuffers()
	{
		commandBuffers.resize(VvtSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vvtDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vvtDevice.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VvtRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(vvtDevice.device(), vvtDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}


	VkCommandBuffer VvtRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Cannot call beginFrame when frame has already started!");

		auto result = vvtSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void VvtRenderer::endFrame()
	{
		assert(isFrameStarted && "Cannot end the frame when there is no current frame in progress!");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = vvtSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vvtWindow.wasWindowResized()) {
			vvtWindow.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VvtSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void VvtRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot begin the render pass when there is no current frame in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vvtSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vvtSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vvtSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vvtSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vvtSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vvtSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VvtRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot end the render pass when there is no current frame in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame!");
		vkCmdEndRenderPass(commandBuffer);
	}

	void VvtRenderer::beginImGuiRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot begin the render pass when there is no current frame in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame!");

		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = vvtSwapChain->getImGuiRenderPass();
		info.framebuffer = vvtSwapChain->getFrameBuffer(currentImageIndex);
		info.renderArea.extent = vvtSwapChain->getSwapChainExtent();
		info.clearValueCount = 1;

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		info.clearValueCount = static_cast<uint32_t>(clearValues.size());
		info.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VvtRenderer::endImGuiRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cannot end the render pass when there is no current frame in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame!");
		vkCmdEndRenderPass(commandBuffer);
	}


}