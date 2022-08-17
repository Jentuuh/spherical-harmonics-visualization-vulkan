#include "vvt_model.hpp"
#include "vvt_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace std {
    template<>
    struct hash<vae::VvtModel::Vertex> {
        size_t operator()(vae::VvtModel::Vertex const& vertex) const 
        {
            size_t seed = 0;
            vae::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace vae {

    VvtModel::VvtModel(VvtDevice& device, const VvtModel::Builder &builder) : vvtDevice{ device } {
        og_vertex_data = builder.vertices;
        old_vertex_data = builder.vertices;
        new_vertex_data = builder.vertices;
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);

        minX = builder.minX;
        maxX = builder.maxX;
        minY = builder.minY;
        maxY = builder.maxY;
        minZ = builder.minZ;
        maxZ = builder.maxZ;
    }

    VvtModel::~VvtModel() {}

    std::unique_ptr<VvtModel> VvtModel::createModelFromFile(VvtDevice& device, const std::string& filePath)
    {
        Builder builder{};
        builder.loadModel(filePath);
        std::cout << "Successfully loaded model with " << builder.vertices.size() << " vertices." << std::endl;
        std::cout << "Min X: " << builder.minX << " Max X: " << builder.maxX << "Min Y: " << builder.minY << " Max Y: " << builder.maxY << "Min Z: " << builder.minZ << " Max Z: " << builder.maxZ << std::endl;
        return std::make_unique<VvtModel>(device, builder);
    }

    void VvtModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        VvtBuffer stagingBuffer{
            vvtDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<VvtBuffer>(
            vvtDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

        vvtDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void VvtModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);
        
        VvtBuffer stagingBuffer{
            vvtDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<VvtBuffer>(
            vvtDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

        vvtDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void VvtModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void VvtModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> VvtModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VvtModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

        // vertex position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        // vertex color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // normal
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        // uv 
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, uv);
        return attributeDescriptions;
    }

    void VvtModel::Builder::loadModel(const std::string& filePath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        minX = std::numeric_limits<float>::max();
        minY = std::numeric_limits<float>::max();
        minZ = std::numeric_limits<float>::max();

        maxX = std::numeric_limits<float>::min();
        maxY = std::numeric_limits<float>::min();
        maxZ = std::numeric_limits<float>::min();
        
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                // Reading vertex position
                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2], 
                    };

                    // Object boundaries
                    if (attrib.vertices[3 * index.vertex_index + 0] < minX) {
                        minX = attrib.vertices[3 * index.vertex_index + 0];
                    }
                    if (attrib.vertices[3 * index.vertex_index + 0] > maxX) {
                        maxX = attrib.vertices[3 * index.vertex_index + 0];
                    }
                    if (attrib.vertices[3 * index.vertex_index + 1] < minY) {
                        minY = attrib.vertices[3 * index.vertex_index + 1];
                    }
                    if (attrib.vertices[3 * index.vertex_index + 1] > maxY) {
                        maxY = attrib.vertices[3 * index.vertex_index + 1];
                    }
                    if (attrib.vertices[3 * index.vertex_index + 2] < minZ) {
                        minZ = attrib.vertices[3 * index.vertex_index + 2];
                    }
                    if (attrib.vertices[3 * index.vertex_index + 2] > maxZ) {
                        maxZ = attrib.vertices[3 * index.vertex_index + 2];
                    }

                    // Vertex color expansion (not supported in .OBJ by default, but tinyobjloader supports it)
                    // The vertex RGB color appears in the .OBJ file right after the vertex position.
                    vertex.color = {
                            attrib.colors[3 * index.vertex_index + 0],
                            attrib.colors[3 * index.vertex_index + 1],
                            attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                // Reading normal
                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                // Reading UV coords
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[3 * index.texcoord_index + 0],
                        attrib.texcoords[3 * index.texcoord_index + 1],
                    };
                }

                // We avoid vertex duplication by using an index buffer 
                // (unordered map checks whether a vertex has already been referenced before)
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
               }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}