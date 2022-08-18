#pragma once

#include "vvt_buffer.hpp"
#include "vvt_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE	// Forces that glm uses depth values in the [0;1] range, unlike the standard [-1;1] range.
#include <glm/glm.hpp>

// std 
#include <memory>
#include <vector>

namespace vvt {
	class VvtModel
	{
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			float minX;
			float maxX;
			float minY;
			float maxY;
			float minZ;
			float maxZ;

			void loadModel(const std::string& filePath);
		};

		VvtModel(VvtDevice &device, const VvtModel::Builder &builder);
		~VvtModel();

		VvtModel(const VvtModel&) = delete;
		VvtModel& operator=(const VvtModel&) = delete;

		float minimumX() { return minX; };
		float maximumX() { return maxX; };
		float minimumY() { return minY; };
		float maximumY() { return maxY; };
		float minimumZ() { return minZ; };
		float maximumZ() { return maxZ; };
		std::vector<Vertex>& getVertices() { return old_vertex_data; };

		static std::unique_ptr<VvtModel> createModelFromFile(VvtDevice& device, const std::string& filePath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t> &indices);

		float minX;
		float maxX;
		float minY;
		float maxY;
		float minZ;
		float maxZ;

		VvtDevice& vvtDevice;

		std::vector<Vertex> og_vertex_data;
		std::vector<Vertex> old_vertex_data;
		std::vector<Vertex> new_vertex_data;

		std::unique_ptr<VvtBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<VvtBuffer> indexBuffer;

		uint32_t indexCount;
	};
}
