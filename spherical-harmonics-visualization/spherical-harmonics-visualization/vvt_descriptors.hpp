#pragma once

#include "vvt_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vvt {

    class VvtDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VvtDevice& vvtDevice) : vvtDevice{ vvtDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VvtDescriptorSetLayout> build() const;

        private:
            VvtDevice& vvtDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VvtDescriptorSetLayout(
            VvtDevice& vvtDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VvtDescriptorSetLayout();
        VvtDescriptorSetLayout(const VvtDescriptorSetLayout&) = delete;
        VvtDescriptorSetLayout& operator=(const VvtDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VvtDevice& vvtDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VvtDescriptorWriter;
    };


    class VvtDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VvtDevice& vvtDevice) : vvtDevice{ vvtDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VvtDescriptorPool> build() const;

        private:
            VvtDevice& vvtDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VvtDescriptorPool(
            VvtDevice& vvtDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VvtDescriptorPool();
        VvtDescriptorPool(const VvtDescriptorPool&) = delete;
        VvtDescriptorPool& operator=(const VvtDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VvtDevice& vvtDevice;
        VkDescriptorPool descriptorPool;

        friend class VvtDescriptorWriter;
    };

    class VvtDescriptorWriter {
    public:
        VvtDescriptorWriter(VvtDescriptorSetLayout& setLayout, VvtDescriptorPool& pool);

        VvtDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VvtDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VvtDescriptorSetLayout& setLayout;
        VvtDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}
