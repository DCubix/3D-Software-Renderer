#ifndef T_TEXTURE_H
#define T_TEXTURE_H

#include <string>

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

	glm::vec4* pixels() { return m_pixels; }

	TTexture(int w, int h);
	TTexture(const std::string& fileName);
	virtual ~TTexture();

	bool valid() const { return m_width != 0 && m_height != 0 && m_pixels != nullptr; }

private:
	glm::vec4* m_pixels;
	int m_width, m_height;
};

#endif // T_TEXTURE_H