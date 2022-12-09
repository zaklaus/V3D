#include "IDevice.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"

static struct {
	sg_shader default_shd;
	sg_pipeline default_pip;
	sg_bindings default_bindings;
	sg_pass_action default_pass_action;

	glm::ivec2 viewport;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
} state;

bool IDevice::init() {
	/* setup sokol_gfx */
	sg_desc desc{};
	sg_setup(&desc);

	
	/* a default shader */
	{
		sg_shader_desc default_shader_desc{};
		default_shader_desc.vs.source =
			"#version 330\n"
			"layout(location=0) in vec3 position;\n"
			"layout(location=1) in vec2 uv;\n"
			"out vec2 texCoords;\n"
			"void main() {\n"
			"  gl_Position = vec4(position, 1.0);\n"
			"  texCoords = uv;\n"
			"}\n";

		default_shader_desc.fs.source =
			"#version 330\n"
			"in vec2 texCoords;\n"
			"out vec4 frag_color;\n"
			"uniform sampler2D texture0;\n"
			"void main() {\n"
			"  frag_color = texture2D(texture0, texCoords);\n"
			"}\n";

		state.default_shd = sg_make_shader(&default_shader_desc);
	}

	/* a pipeline state object (default render states are fine for triangle) */
	{
		sg_pipeline_desc default_pip_desc{};
		
		sg_layout_desc defailt_pip_layout_desc{};
		defailt_pip_layout_desc.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		defailt_pip_layout_desc.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
		
		default_pip_desc.layout = defailt_pip_layout_desc;
		default_pip_desc.shader = state.default_shd;
		default_pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;

		state.default_pip = sg_make_pipeline(&default_pip_desc);
	}

	/* default pass action */
	{
		sg_pass_action default_pass{};

		sg_color_attachment_action attachment_color{};
		attachment_color.action = SG_ACTION_CLEAR;
		attachment_color.value = { 0.25f, 0.5f, 0.75f, 1.0f };
		default_pass.colors[0] = attachment_color;
		state.default_pass_action = default_pass;
	}

	return true;
}

void IDevice::destroy() {
	sg_shutdown();
}

Image createImage(const ImageDesc& imageDesc) {
	return sg_make_image(imageDesc);
}

void destroyImage(Image& imageHandle) {
	sg_destroy_image(imageHandle);
	imageHandle.id = SG_INVALID_ID;
}

void bindImage(const Image& imageHandle, int samplerId) {
	state.default_bindings.fs_images[samplerId] = imageHandle;
}

Buffer createBuffer(const BufferDesc& bufferDesc) {
	return sg_make_buffer(bufferDesc);
}

void destroyBuffer(Buffer& bufferHandle) {
	sg_destroy_buffer(bufferHandle);
	bufferHandle.id = SG_INVALID_ID;
}

void bindVertexBuffer(const Buffer& bufferHandle) {
	state.default_bindings.vertex_buffers[0] = bufferHandle;
}

void bindIndexBuffer(const Buffer& bufferHandle) {
	state.default_bindings.index_buffer = bufferHandle;
}

void IDevice::setViewport(const glm::ivec2& size) {
	state.viewport = size;
}

void IDevice::setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj) {
	state.view = view;
	state.proj = proj;
}

void IDevice::setModelMatrix(const glm::mat4& model) {
	state.model = model;
}

void IDevice::beginPass() {
	sg_begin_default_pass(&state.default_pass_action, state.viewport.x, state.viewport.y);
	sg_apply_pipeline(state.default_pip);
	sg_apply_bindings(&state.default_bindings);
}

void IDevice::endPass() {
	sg_end_pass();
}

void IDevice::present() {
	sg_commit();
}

/* --- */

IDevice* createDevice() {
	return new IDevice();
}