#include "TTexture.h"

#include <algorithm>

TTexture::TTexture(int w, int h) {
	m_width = w;
	m_height = h;
	m_pixels = new glm::vec4[w * h];
	std::fill_n(m_pixels, w * h, glm::vec4(0.0f));
}

TTexture::TTexture(const std::string& fileName) {
	int w, h, comp;
	stbi_uc* pixels = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb);
	if (pixels) {
		m_width = w;
		m_height = h;
		m_pixels = new glm::vec4[w * h];
		std::fill_n(m_pixels, w * h, glm::vec4(1.0f));
		
		for (int i = 0; i < w*h; i++) {
			int j = i*comp;
			for (int k = 0; k < comp; k++) {
				m_pixels[i][k] = float(pixels[j + k]) / 255.0f;
			}
		}

	} else {
		m_pixels = nullptr;
		m_width = 0;
		m_height = 0;
	}
}

TTexture::~TTexture() {
	if (m_pixels != nullptr) {
		delete[] m_pixels;
		m_pixels = nullptr;
	}
}

glm::vec4 TTexture::get(int x, int y) const {
	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	return m_pixels[x + y * m_width];
}

glm::vec4 TTexture::get(float s, float t) const {
	s = s * (float(m_width) - 0.5f);
	t = t * (float(m_height) - 0.5f);
	int x = floor(s);
	int y = floor(t);
	return get(x, y);
}

glm::vec4 TTexture::getBilinear(float s, float t) const {
	s = s * (float(m_width) - 0.5f);
	t = t * (float(m_height) - 0.5f);
	int x = floor(s);
	int y = floor(t);

	float s_ratio = s - x;
	float t_ratio = t - y;
	float s_opposite = 1.0f - s_ratio;
	float t_opposite = 1.0f - t_ratio;

	glm::vec4 res = (get(x, y) * s_opposite + get(x+1, y) * s_ratio) * t_opposite + 
					(get(x, y+1) * s_opposite + get(x+1, y+1) * s_ratio) * t_ratio;
	return res;
}

void TTexture::set(int x, int y, const glm::vec4& color) {
	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return;
	}
	if (color.a <= 0.0f) {
		return;
	}
	int index = x + y * m_width;
	glm::vec4 newPixel = glm::mix(m_pixels[index], color, color.a);
	m_pixels[index].r = newPixel.r;
	m_pixels[index].g = newPixel.g;
	m_pixels[index].b = newPixel.b;
}