#include "TGfx.h"

#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>

void showError() {
	std::cerr << SDL_GetError() << std::endl;
}

std::optional<GFX> GFX::create(const std::string title, int width, int height, float downScale) {
	downScale = downScale < 1.0f ? 1.0f : downScale;

	int tw = int(width / downScale);
	int th = int(height / downScale);

	GFX gfx;
	SDL_Init(SDL_INIT_VIDEO);

	gfx.m_window = SDL_CreateWindow(
		"TRender",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_SHOWN
	);
	if (gfx.m_window == nullptr) {
		showError();
		return {};
	}

	gfx.m_renderer = SDL_CreateRenderer(gfx.m_window, 0, SDL_RENDERER_ACCELERATED);
	if (gfx.m_renderer == nullptr) {
		showError();
		return {};
	}

	gfx.m_screenBuffer = SDL_CreateTexture(
		gfx.m_renderer,
		SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING,
		tw, th
	);

	gfx.m_shouldClose = false;
	gfx.m_windowWidth = width;
	gfx.m_windowHeight = height;
	gfx.m_drawWidth = tw;
	gfx.m_drawHeight = th;
	gfx.m_modelMatrixStack.loadIdentity();
	gfx.m_projectionMatrixStack.loadIdentity();
	gfx.m_viewportMatrix = glm::mat4(
		glm::vec4(tw / 2,	0.0f, 0.0f, tw / 2),
		glm::vec4(	0.0f, th / 2, 0.0f, th / 2),
		glm::vec4(	0.0f,	0.0f, 0.0f, 0.0f),
		glm::vec4(	0.0f, 	0.0f, 0.0f, 0.0f)
	);

	gfx.m_pixels = new glm::vec3[tw * th];
	std::memset(gfx.m_pixels, 0, sizeof(glm::vec3) * tw * th);

	return std::make_optional(gfx);
}

void GFX::update() {
	while (SDL_PollEvent(&m_event)) {
		if (m_event.type == SDL_QUIT) m_shouldClose = true;
	}

	/// Flip screen
	Uint8* pixels;
	int pitch;
	SDL_LockTexture(m_screenBuffer, nullptr, (void**) &pixels, &pitch);
	for (int y = 0; y < m_drawHeight; y++) {
		for (int x = 0; x < m_drawWidth; x++) {
			int index = x + y * m_drawWidth;
			glm::vec3 col = m_pixels[index];
			int pidx = index * 3;
			pixels[pidx + 0] = int(col.r * 255);
			pixels[pidx + 1] = int(col.g * 255);
			pixels[pidx + 2] = int(col.b * 255);
		}
	}
	SDL_UnlockTexture(m_screenBuffer);

	SDL_RenderClear(m_renderer);

	SDL_Rect rec = { 0, 0, m_windowWidth, m_windowHeight };
	SDL_RenderCopy(m_renderer, m_screenBuffer, nullptr, &rec);

	SDL_RenderPresent(m_renderer);
}

void GFX::destroy() {
	SDL_DestroyTexture(m_screenBuffer);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	delete[] m_pixels;
}

void GFX::clear(glm::vec3 color) {
	std::fill_n(m_pixels, m_drawWidth * m_drawHeight, color);
}

void GFX::pixel(int x, int y, glm::vec3 color) {
	if (x < 0 || x >= m_drawWidth || y < 0 || y >= m_drawHeight) {
		return;
	}
	int index = x + y * m_drawWidth;
	m_pixels[index].r = color.r;
	m_pixels[index].g = color.g;
	m_pixels[index].b = color.b;
}

void GFX::line(int x1, int y1, int x2, int y2, glm::vec3 color) {
	int dx = std::abs(x2 - x1);
	int sx = x1 < x2 ? 1 : -1;
	int dy = -std::abs(y2 - y1);
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;
	int e2 = 0;
	
	int x = x1;
	int y = y1;

	while (true) {
		pixel(x, y, color);

		if (x == x2 && y == y2) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
	}
}

void GFX::line(const TVertex& v0, const TVertex& v1) {
	TVertex vt0 = transformVertex(v0);
	TVertex vt1 = transformVertex(v1);

	int x1 = int(vt0.position.x);
	int y1 = int(vt0.position.y);
	int x2 = int(vt1.position.x);
	int y2 = int(vt1.position.y);
	line(x1, y1, x2, y2, v0.color);
}

static float perpDot(const glm::vec2& a, const glm::vec2& b) {
	return a.x * b.y - a.y * b.x;
}

static glm::vec3 interpolateNormals(const glm::vec3 v1N, const glm::vec3 nE0, const glm::vec3 nE1, float s, float t) {
	return v1N + (s * nE0) + (t * nE1);
}

static glm::vec4 interpolateColors(const glm::vec4 v1C, const glm::vec4 dE0, const glm::vec4 dE1, float s, float t) {
	return v1C + (s * dE0) + (t * dE1);
}

void GFX::triangle(const TVertex& v0, const TVertex& v1, const TVertex& v2) {
	TVertex vt0 = transformVertex(v0);
	TVertex vt1 = transformVertex(v1);
	TVertex vt2 = transformVertex(v2);

	glm::vec4 _vs1 = vt1.position - vt0.position;
	glm::vec4 _vs2 = vt2.position - vt0.position;

	glm::vec3 eye = glm::vec3(modelView().matrix()[3]);
	glm::vec3 tV = glm::normalize(eye - glm::vec3(vt0.position));
	glm::vec3 tN = glm::cross(glm::vec3(_vs1), glm::vec3(_vs2));

	if (glm::dot(tN, tV) <= 0.0f) {
		return;
	}

	int maxX = std::max(vt0.position.x, std::max(vt1.position.x, vt2.position.x));
	int minX = std::min(vt0.position.x, std::min(vt1.position.x, vt2.position.x));
	int maxY = std::max(vt0.position.y, std::max(vt1.position.y, vt2.position.y));
	int minY = std::min(vt0.position.y, std::min(vt1.position.y, vt2.position.y));

	glm::vec2 vs1(_vs1.x, _vs1.y);
	glm::vec2 vs2(_vs2.x, _vs2.y);

	glm::vec4 dcolorE0 = v1.color - v0.color;
	glm::vec4 dcolorE1 = v2.color - v0.color;

	glm::vec2 q;
	for (int x = minX; x <= maxX; x++) {
		for (int y = minY; y <= maxY; y++) {
			q.x = x - vt0.position.x;
			q.y = y - vt0.position.y;

			float s = perpDot(q, vs2) / perpDot(vs1, vs2);
			float t = perpDot(vs1, q) / perpDot(vs1, vs2);

			if ((s >= 0) && (t >= 0) && (s + t <= 1)) {
				glm::vec4 col = interpolateColors(v0.color, dcolorE0, dcolorE1, s, t);
				pixel(x, y, col);
			} else { continue; }
		}
	}
}

TVertex GFX::transformVertex(const TVertex& v) {
	TVertex vt = v.transform(projection().matrix(), modelView().matrix());
	vt.position /= vt.position.w;
	vt.position = vt.position * m_viewportMatrix;
	return vt;
}
