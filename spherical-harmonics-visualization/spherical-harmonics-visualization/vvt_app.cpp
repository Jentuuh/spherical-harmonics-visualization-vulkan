#include "vvt_app.hpp"
#include "vvt_buffer.hpp"

// std
#include <cassert>
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <filesystem>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace vvt {

	struct GlobalUbo {
		alignas(16) glm::mat4 projection{ 1.0f };
		alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
		alignas(16) glm::mat4 view{ 1.0f };
	};

	VvtApp::VvtApp()
	{
		loadTextures();

		globalPool = VvtDescriptorPool::Builder(vvtDevice)
			.setMaxSets(2 * VvtSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * VvtSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * VvtSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		initDescriptorsAndUBOs();

		initImgui();
		loadGameObjects();
		initVisualizations();

		viewerObject = std::make_unique<VvtGameObject>(VvtGameObject::createGameObject());

		// Change initial position to provide better overview of the SH functions
		viewerObject->transform.translation = { 10.0f, 0.0f, -40.0f };
	}

	VvtApp::~VvtApp(){
		vkDestroyDescriptorPool(vvtDevice.device(), imGuiPool, nullptr);
	}

	void VvtApp::run()
	{

		simpleRenderSystem = std::make_unique<SimpleRenderSystem>(
			vvtDevice, 
			vvtRenderer.getSwapChainRenderPass(),
			vvtRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout());
	
        auto currentTime = std::chrono::high_resolution_clock::now();

		// ImGui state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		while (!vvtWindow.shouldClose())
		{
			glfwPollEvents();

			renderImGuiWindow();

            // Time step (delta time)
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			updateCamera(frameTime);

			// Render loop
			if (auto commandBuffer = vvtRenderer.beginFrame()) {
				int frameIndex = vvtRenderer.getFrameIndex();
				
				// Update phase
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render Scene
				vvtRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem->renderGameObjects(
					commandBuffer, 
					globalDescriptorSets[frameIndex], 
					gameObjects,
					sphereFunctions,
					camera, 
					frameTime,
					viewerObject.get());
				vvtRenderer.endSwapChainRenderPass(commandBuffer);

				// Draw ImGui Window
				vvtRenderer.beginImGuiRenderPass(commandBuffer);
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
				vvtRenderer.endImGuiRenderPass(commandBuffer);

				vvtRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(vvtDevice.device());
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void VvtApp::initImgui()
	{
		// Create descriptor pool for ImGui
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;


		if (vkCreateDescriptorPool(vvtDevice.device(), &pool_info, nullptr, &imGuiPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool for imgui!");
		}

		// Init ImGui library
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// ImGui style
		ImGui::StyleColorsDark();

		// Platform/renderer bindings
		ImGui_ImplGlfw_InitForVulkan(vvtWindow.getGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vvtDevice.getInstance();
		init_info.PhysicalDevice = vvtDevice.getPhysicalDevice();
		init_info.Device = vvtDevice.device();
		init_info.Queue = vvtDevice.graphicsQueue();
		init_info.DescriptorPool = imGuiPool;
		init_info.MinImageCount = VvtSwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = VvtSwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		// ImGui Vulkan initialization
		ImGui_ImplVulkan_Init(&init_info, vvtRenderer.getImGuiRenderPass());

		// Upload fonts to GPU
		VkCommandBuffer command_buffer = vvtDevice.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		vvtDevice.endSingleTimeCommands(command_buffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}


	/* Initializer for game objects that are always loaded into the scene */
	void VvtApp::loadGameObjects()
    {
		// Example model
		//std::shared_ptr<VvtModel> exampleModel = VvtModel::createModelFromFile(vvtDevice, "../Models/cube.obj");

		//auto exampleCube = VvtGameObject::createGameObject();
		//exampleCube.modelPath = std::string("../Models/cube.obj");
		//exampleCube.model = exampleModel;
		//exampleCube.setPosition({ .0f, .0f, 2.0f });
		//exampleCube.transform.rotation = { .0f, .0f, .0f };
		//exampleCube.setScale({ .5f, .5f, .5f });
		//gameObjects.push_back(std::move(exampleCube));
	}

	/* Initialize textures */
	void VvtApp::loadTextures()
	{
		const char* test = "../Textures/pepe.jpg";
		testTexture = std::make_unique<VvtTexture>(vvtDevice, test, TEXTURE_TYPE_STANDARD_2D);
	}


	void VvtApp::initDescriptorsAndUBOs()
	{
		// Create UBOs
		uboBuffers.resize(VvtSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VvtBuffer>(
				vvtDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			uboBuffers[i]->map();
		}

		// Create set layouts
		 globalSetLayout = std::move(VvtDescriptorSetLayout::Builder(vvtDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)	// UBO
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)	// Texture sampler
			.build());

		// Write to descriptor sets
		globalDescriptorSets.resize(VvtSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			auto textureInfo = testTexture->descriptorInfo();

			// Write buffer info to binding 0, texture info to binding 1
			VvtDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &textureInfo)
				.build(globalDescriptorSets[i]);
		}
	}

	void VvtApp::initVisualizations()
	{
		std::shared_ptr<VvtModel> pointModel = VvtModel::createModelFromFile(vvtDevice, "../Models/sphere.obj");

		sh::SphericalFunction func = [](double phi, double theta) { return glm::sin(phi) * glm::cos(phi); };
		SphereContainer sphereFunc1 = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f }, 3.0f, func, pointModel };
		sphereFunctions.push_back(sphereFunc1);
		sphereFunctions.back().generateSpherePoints();
	}



	void VvtApp::renderImGuiWindow()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("SH Visualizer UI");

		/*
		* Render ImGui stuff here
		*/
		
		ImGui::BeginTabBar("Menu");

		if (ImGui::BeginTabItem("Info"))
		{
			ImGui::TextWrapped("This tool can be used to play around with and visualize SH basis functions and the impact of their coefficients on the reconstruction of an arbitrary spherical function. \
			\n\n The reconstruction of the original spherical function by taking the sum of the basis functions each weighted by their coefficient is shown on the outer left. To the right of that, the original spherical function is shown. Lastly, the SH basis functions are visualized. Each column represents an order (starting from 0). The columns are ordered by incrementing degree (e.g. [-1, 0, 1]).");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Settings"))
		{
			if (ImGui::DragFloat3("Rotation (Euler XYZ)", glm::value_ptr(sphereFunctions[0].getRotation()), 0.01f, 0.0f, 2 * glm::pi<float>(), "%.2f"))
			{
				sphereFunctions[0].updateRotation();
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();
		ImGui::Render();
	}


	/* Update camera view/model matrix */
	void VvtApp::updateCamera(float frameTime)
	{
		float aspect = vvtRenderer.getAspectRatio();

		// Update camera model (game object that contains camera
		cameraController.moveInPlaneXZ(vvtWindow.getGLFWwindow(), frameTime, *viewerObject);
		// Update camera view matrix
		camera.setViewYXZ(viewerObject->transform.translation, viewerObject->transform.rotation);
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
	}
}