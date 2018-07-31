#ifndef T_GFX_H
#define T_GFX_H

#include <string>
#include <optional>
#include <array>

#include "SDL2/SDL.h"
#include "vec3.hpp"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp"

#include "TMatrixStack.h"
#include "../data/TStructs.h"
#include "../data/TFrameBuffer.h"

#define T_MAX_MATRIX_TACK_DEPTH 128

class GFX {
public:
	GFX() {}
	virtual ~GFX() {}

	static std::optional<GFX> create(const std::string title, int width, int height, float downScale=1.0f);
	void destroy();

	bool shouldClose() const { return m_shouldClose; }
	void flip();
	void poll();
	double time() const { return double(SDL_GetTicks()) / 1000.0; }

	/// Drawing Functions
	void clear(glm::vec3 color = { 0.0f, 0.0f, 0.0f });
	void pixel(int x, int y, glm::vec4 color);
	void line(int x1, int y1, int x2, int y2, glm::vec4 color);

	/// 3D drawing
	void triangle(const TVertex& v0, const TVertex& v1, const TVertex& v2);
	void triangleUC(const TVertex& v0, const TVertex& v1, const TVertex& v2);

	TMatrixStack& modelView() { return m_modelMatrixStack; }
	TMatrixStack& projection() { return m_projectionMatrixStack; }

	TFrameBuffer* target() {
		if (m_target == nullptr)
			return m_defaultTarget;
		return m_target;
	}

	void target(TFrameBuffer* target) { m_target = target; }

	TTexture* boundTexture() { return m_boundTexture; }
	void boundTexture(TTexture* tex) { m_boundTexture = tex; }

	TShader* boundShader() { 
		if (m_boundShader == nullptr)
			return g_defaultShader;
		return m_boundShader;
	}
	void boundShader(TShader* shader) { m_boundShader = shader; }

private:
	bool m_shouldClose;

	int m_windowWidth, m_windowHeight,
		m_drawWidth, m_drawHeight;

	SDL_Event m_event;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_screenBuffer;

	TTexture* m_boundTexture;
	TShader* m_boundShader;

	TFrameBuffer* m_defaultTarget;
	TFrameBuffer* m_target;

	TMatrixStack m_modelMatrixStack, m_projectionMatrixStack;
	glm::mat4 m_viewportMatrix;

	static TShader* g_defaultShader;
};

#endif // T_GFX_H