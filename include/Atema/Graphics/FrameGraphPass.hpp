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

#ifndef ATEMA_GRAPHICS_FRAMEGRAPHPASS_HPP
#define ATEMA_GRAPHICS_FRAMEGRAPHPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/DepthStencil.hpp>
#include <Atema/Graphics/Enums.hpp>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace at
{
	class FrameGraphContext;

	class ATEMA_GRAPHICS_API FrameGraphPass
	{
		friend class FrameGraphBuilder;

	public:
		using ExecutionCallback = std::function<void(FrameGraphContext&)>;

		FrameGraphPass() = delete;
		~FrameGraphPass();

		const std::string& getName() const noexcept;

		// If enabled, rendering happens on RenderFrame instead of output textures
		// Default : false
		void enableRenderFrameOutput(bool enable);

		// If enabled, this pass must only use secondary command buffers
		// Default : false
		void enableSecondaryCommandBuffers(bool enable);

		// Sets the callback function that will be called by the FrameGraph during execution
		void setExecutionCallback(const ExecutionCallback& callback);

		// Specifies a texture that will be sampled during some shader stages
		void addSampledTexture(FrameGraphTextureHandle textureHandle, Flags<ShaderStage> shaderStages);
		// Specifies a texture that will be used as an input attachment bound at the required index
		void addInputTexture(FrameGraphTextureHandle textureHandle, uint32_t index);
		// Specifies a texture that will be used as an output attachment bound at the required index
		void addOutputTexture(FrameGraphTextureHandle textureHandle, uint32_t index);
		// Specifies a texture that will be used as an output attachment bound at the required index
		// Before this pass is called, the texture will be cleared using the desired color
		void addOutputTexture(FrameGraphTextureHandle textureHandle, uint32_t index, const Color& clearColor);
		// Specifies a texture that will be used as a depth attachment
		void setDepthTexture(FrameGraphTextureHandle textureHandle);
		// Specifies a texture that will be used as a depth attachment
		// Before this pass is called, the texture will be cleared using the desired depth/stencil
		void setDepthTexture(FrameGraphTextureHandle textureHandle, const DepthStencil& depthStencil);

		bool useRenderFrameOutput() const noexcept;

		bool useSecondaryCommandBuffers() const noexcept;

		const ExecutionCallback& getExecutionCallback() const noexcept;

		const std::vector<FrameGraphTextureHandle>& getSampledTextures() const noexcept;
		const std::vector<FrameGraphTextureHandle>& getInputTextures() const noexcept;
		const std::vector<FrameGraphTextureHandle>& getOutputTextures() const noexcept;
		const std::vector<FrameGraphTextureHandle>& getClearedTextures() const noexcept;
		FrameGraphTextureHandle getDepthTexture() const noexcept;

		uint32_t getInputIndex(FrameGraphTextureHandle textureHandle) const;
		uint32_t getOutputIndex(FrameGraphTextureHandle textureHandle) const;
		Flags<ShaderStage> getSamplingStages(FrameGraphTextureHandle textureHandle) const;

		const Vector2u& getOutputSize() const noexcept;

	private:
		FrameGraphPass(const FrameGraphBuilder& frameGraphBuilder, const std::string& name);

		static void checkFlagsCompatibility(Flags<TextureUsage> usages1, Flags<TextureUsage> usages2);
		void registerTexture(FrameGraphTextureHandle textureHandle, Flags<TextureUsage> usages);
		bool validateSize(FrameGraphTextureHandle textureHandle, Vector2u& size);

		const FrameGraphBuilder& m_frameGraphBuilder;

		std::string m_name;

		bool m_useRenderFrameOutput;

		bool m_useSecondaryCommandBuffers;

		ExecutionCallback m_executionCallback;

		std::unordered_map<FrameGraphTextureHandle, Flags<TextureUsage>> m_textureUsages;

		Vector2u m_inputSize;
		Vector2u m_outputSize;

		std::vector<FrameGraphTextureHandle> m_sampledTextures;
		std::vector<FrameGraphTextureHandle> m_inputTextures;
		std::vector<FrameGraphTextureHandle> m_outputTextures;
		FrameGraphTextureHandle m_depthTexture;

		std::vector<FrameGraphTextureHandle> m_clearedTextures;
		std::unordered_map<FrameGraphTextureHandle, Color> m_clearColors;
		std::unordered_map<FrameGraphTextureHandle, DepthStencil> m_clearDepths;

		std::unordered_map<FrameGraphTextureHandle, uint32_t> m_inputIndices;
		std::unordered_map<FrameGraphTextureHandle, uint32_t> m_outputIndices;

		std::unordered_map<FrameGraphTextureHandle, Flags<ShaderStage>> m_samplingStages;
	};
}

#endif
