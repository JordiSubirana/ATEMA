/*
	Copyright 2022 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_GRAPHICS_FRAMEGRAPHBUILDER_HPP
#define ATEMA_GRAPHICS_FRAMEGRAPHBUILDER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Graphics/FrameGraphPass.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Graphics/Enums.hpp>
#include <Atema/Renderer/Image.hpp>

#include <string>
#include <unordered_set>
#include <vector>

namespace at
{
	class RenderPass;
	class Image;

	class ATEMA_GRAPHICS_API FrameGraphBuilder
	{
	public:
		FrameGraphBuilder();
		~FrameGraphBuilder();

		FrameGraphTextureHandle createTexture(const FrameGraphTextureSettings& settings);
		FrameGraphTextureHandle importTexture(const Ptr<Image>& image, uint32_t layer = 0, uint32_t mipLevel = 0);

		FrameGraphPass& createPass(const std::string& name);

		Ptr<FrameGraph> build();

		const FrameGraphTextureSettings& getTextureSettings(FrameGraphTextureHandle textureHandle) const;
		
	private:
		static constexpr size_t InvalidPassIndex = std::numeric_limits<size_t>::max();

		struct PhysicalTexture;

		struct PassRange
		{
			size_t first = InvalidPassIndex;
			size_t last = InvalidPassIndex;

			bool isInside(size_t index) const noexcept
			{
				return first <= index && index <= last;
			}

			bool overlap(const PassRange& other) const noexcept
			{
				return other.isInside(first) || other.isInside(last) || isInside(other.first) || isInside(other.last);
			}

			bool operator==(const PassRange& other) const noexcept
			{
				return first == other.first && last == other.last;
			}

			bool operator<(const PassRange& other) const noexcept
			{
				return first < other.first;
			}

			bool operator>(const PassRange& other) const noexcept
			{
				return last > other.last;
			}

			bool operator<=(const PassRange& other) const noexcept
			{
				if (*this == other)
					return true;

				return *this < other;
			}

			bool operator>=(const PassRange& other) const noexcept
			{
				if (*this == other)
					return true;

				return *this > other;
			}
		};

		struct TextureData
		{
			FrameGraphTextureHandle textureHandle = FrameGraph::InvalidTextureHandle;
			bool used = false;
			bool imported = false;
			bool finalOutput = false;
			std::vector<size_t> sampled;
			std::vector<size_t> input;
			std::vector<size_t> output;
			std::vector<size_t> depth;
			std::vector<size_t> clear;
			PassRange readRange;
			PassRange writeRange;
			PassRange useRange;

			std::vector<Flags<TextureUsage>> usages;

			PhysicalTexture* physicalTexture = nullptr;

			size_t nextRead(size_t passIndex) const;
			size_t nextWrite(size_t passIndex) const;
			size_t nextClear(size_t passIndex) const;
			size_t nextUse(size_t passIndex) const;
			bool doClear(size_t passIndex) const;
		};

		struct PassData
		{
			bool used = false;
			FrameGraphPass* pass = nullptr;
			std::unordered_set<size_t> dependencies;
		};

		struct PhysicalTextureAlias
		{
			FrameGraphTextureHandle textureHandle = FrameGraph::InvalidTextureHandle;

			bool imported = false;

			Image::Settings imageSettings;

			PassRange range;
		};

		struct PhysicalTexture
		{
			bool imported = false;

			Image::Settings imageSettings;

			Ptr<Image> image;
			Ptr<ImageView> imageView;

			std::vector<FrameGraph::TextureBarrier> barriers;

			std::vector<FrameGraphTextureHandle> textureHandles;
			std::vector<PassRange> ranges;

			bool isCompatible(const PhysicalTextureAlias& alias) const
			{
				if (imported || alias.imported)
					return false;

				for (auto& range : ranges)
				{
					if (alias.range.overlap(range))
						return false;
				}

				if (imageSettings.width != alias.imageSettings.width)
					return false;

				if (imageSettings.height != alias.imageSettings.height)
					return false;

				if (imageSettings.format != alias.imageSettings.format)
					return false;

				if (imageSettings.mipLevels != alias.imageSettings.mipLevels)
					return false;

				if (imageSettings.samples != alias.imageSettings.samples)
					return false;

				if (imageSettings.tiling != alias.imageSettings.tiling)
					return false;

				return (imageSettings.usages & alias.imageSettings.usages) == alias.imageSettings.usages;
			}

			void insert(const PhysicalTextureAlias& alias)
			{
				int index = 0;
				for (auto& range : ranges)
				{
					if (range > alias.range)
						break;

					index++;
				}

				ranges.emplace(ranges.begin() + index, alias.range);
				textureHandles.emplace(textureHandles.begin() + index, alias.textureHandle);
			}
		};

		struct PhysicalPass
		{
			bool useRenderFrameOutput;
			Ptr<RenderPass> renderPass;
			Ptr<Framebuffer> framebuffer;
			std::vector<CommandBuffer::ClearValue> clearValues;
		};

		void clearTempData();

		void createRenderFrameOutput();
		bool isRenderFrameOutput(FrameGraphTextureHandle textureHandle) const noexcept;

		void createTextureDatas();
		void createPassDatas();
		void setPassUsed(size_t passIndex);
		void checkPassDependencies(size_t passIndex, size_t dependencyIndex, std::unordered_set<size_t>& processedPassIndices);
		bool dependsOn(size_t passIndex1, size_t passIndex2) const noexcept;
		void orderPasses();
		void updateTextureDatas();

		void createPhysicalTextureAliases();
		void createPhysicalTextures();
		void createPhysicalPasses();

		std::vector<FrameGraphTextureSettings> m_textures;
		std::vector<Ptr<FrameGraphPass>> m_passes;

		FrameGraphTextureHandle m_renderFrameColorTextureHandle;
		FrameGraphTextureHandle m_renderFrameDepthTextureHandle;

		std::unordered_map<FrameGraphTextureHandle, Ptr<Image>> m_importedTextures;
		std::unordered_map<FrameGraphTextureHandle, Ptr<ImageView>> m_importedViews;

		// FrameGraph building parameters (invalidated when calling build())
		std::vector<TextureData> m_textureDatas;
		std::vector<PassData> m_passDatas;

		// FrameGraph physical parameters
		std::vector<PhysicalTextureAlias> m_physicalTextureAliases;
		std::vector<Ptr<PhysicalTexture>> m_physicalTextures;
		std::vector<PhysicalPass> m_physicalPasses;
	};
}

#endif
