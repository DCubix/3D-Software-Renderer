#ifndef T_TEXTURE_H
#define T_TEXTURE_H

#include <string>
#include <vector>

#include "vec3.hpp"
#include "vec4.hpp"
#include "gtc/matrix_transform.hpp"

#include "../stb/stb_image.h"

class TTexture {
	friend class GFX;
	friend class TFrameBuffer;
public:
	int width() const { return m_width; }
	int height() const { return m_height; }

	glm::vec4 get(int x, int y) const;
	glm::vec4 get(float s, float t) const;
	glm::vec4 getBilinear(float s, float t) const;
	void set(int x, int y, const glm::vec4& color);

	std::vector<glm::vec4>& pixels() { return m_pixels; }

	TTexture(int w, int h);
	TTexture(const std::string& fileName);
	virtual ~TTexture();

	bool valid() const { return m_width != 0 && m_height != 0 && !m_pixels.empty(); }

	static uint32_t calcZOrder(uint16_t xPos, uint16_t yPos);

protected:
	void clear(glm::vec4 color);

private:
	std::vector<glm::vec4> m_pixels;
	int m_width, m_height, m_size;
};

#endif // T_TEXTURE_H