#pragma once
#include "vvt_renderer.hpp"
#include "vvt_device.hpp"
#include "vvt_window.hpp"
#include "vvt_game_object.hpp"
#include "vvt_descriptors.hpp"
#include "vvt_camera.hpp"
#include "vvt_texture.hpp"
#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"
#include "sphere_container.hpp"


// std 
#include <memory>
#include <vector>
#include <fstream>

namespace vvt {
	class VvtApp
	{
	public:
		const float MAX_FRAME_TIME = .1f;
		static constexpr int WIDTH = 1200;
		static constexpr int HEIGHT = 900;

		VvtApp();
		~VvtApp();

		VvtApp(const VvtApp&) = delete;
		VvtApp& operator=(const VvtApp&) = delete;

		void run();
		void initImgui();

	private:
		void loadGameObjects();
		void loadTextures();
		void initDescriptorsAndUBOs();

		void initVisualizations();

		void renderImGuiWindow();

		void updateCamera(float frameTime);

		VvtWindow vvtWindow{ WIDTH, HEIGHT, "Experimental Visualizations" };
		VvtDevice vvtDevice{ vvtWindow };
		VvtRenderer vvtRenderer{ vvtWindow, vvtDevice };
		VvtCamera camera;
		std::unique_ptr<SimpleRenderSystem> simpleRenderSystem;
		std::unique_ptr<VvtGameObject> viewerObject{};

		// Order of declarations matter!
		std::unique_ptr<VvtDescriptorPool> globalPool{};
		VkDescriptorPool imGuiPool;	 // TODO: make use of VvtDescriptorPool class!

		std::unique_ptr<VvtTexture> testTexture;
		std::vector<std::unique_ptr<VvtBuffer>> uboBuffers;
		std::unique_ptr<VvtDescriptorSetLayout> globalSetLayout;
		std::vector<VkDescriptorSet> globalDescriptorSets;

		std::vector<VvtGameObject> gameObjects;
		std::vector<SphereContainer> sphereFunctions;

		KeyboardMovementController cameraController;
	};
}

