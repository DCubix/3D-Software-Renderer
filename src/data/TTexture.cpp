#include "TTexture.h"

#include <algorithm>

TTexture::TTexture(int w, int h) {
	m_width = w;
	m_height = h;
	m_size = std::max(m_width, m_height);
	m_pixels.resize(m_size * m_size);
	std::fill(m_pixels.begin(), m_pixels.end(), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

TTexture::TTexture(const std::string& fileName) {
	int w, h, comp;
	stbi_uc* pixels = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb);
	if (pixels) {
		m_width = w;
		m_height = h;
		m_size = std::max(m_width, m_height);
		m_pixels.resize(m_size * m_size);
		std::fill(m_pixels.begin(), m_pixels.end(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		for (int y = 0; y < m_height; y++) {
			for (int x = 0; x < m_width; x++) {
				int i = (x + (m_height - 1 - y) * m_width) * comp;
				glm::vec4 col(0.0f, 0.0f, 0.0f, 1.0f);
				for (int k = 0; k < comp; k++) {
					col[k] = pixels[i + k] / 255.0f;
				}
				set(x, y, col);
			}
		}
		stbi_image_free(pixels);
	} else {
		m_width = 0;
		m_height = 0;
	}
}

TTexture::~TTexture() {
}

uint32_t TTexture::calcZOrder(uint16_t xPos, uint16_t yPos) {
	static const uint32_t MASKS[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
	static const uint32_t SHIFTS[] = {1, 2, 4, 8};

	uint32_t x = xPos;  // Interleave lower 16 bits of x and y, so the bits of x
	uint32_t y = yPos;  // are in the even positions and bits from y in the odd;

	x = (x | (x << SHIFTS[3])) & MASKS[3];
	x = (x | (x << SHIFTS[2])) & MASKS[2];
	x = (x | (x << SHIFTS[1])) & MASKS[1];
	x = (x | (x << SHIFTS[0])) & MASKS[0];

	y = (y | (y << SHIFTS[3])) & MASKS[3];
	y = (y | (y << SHIFTS[2])) & MASKS[2];
	y = (y | (y << SHIFTS[1])) & MASKS[1];
	y = (y | (y << SHIFTS[0])) & MASKS[0];

	const uint32_t result = x | (y << 1);
	return result;
}

glm::vec4 TTexture::get(int x, int y) const {
	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	// return m_pixels[calcZOrder(x, y)];
	return m_pixels[x+y*m_width];
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
	int index = x+y*m_width;//calcZOrder(x, y);
	glm::vec4 newPixel = glm::mix(m_pixels[index], color, color.a);
	m_pixels[index].r = newPixel.r;
	m_pixels[index].g = newPixel.g;
	m_pixels[index].b = newPixel.b;
	m_pixels[index].a = newPixel.a;
}

void TTexture::clear(glm::vec4 color) {
	std::fill(m_pixels.begin(), m_pixels.end(), color);
}