#ifndef T_FRAMEBUFFER_H
#define T_FRAMEBUFFER_H

#include "TTexture.h"

class TFrameBuffer {
	friend class GFX;
public:
	float depth(int x, int y) const;
	void depth(int x, int y, float d);

	TTexture* texture() { return m_texture; }

	int width() const { return m_texture->width(); }
	int height() const { return m_texture->height(); }

	void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 0.0f });

	TFrameBuffer(int w, int h);
	virtual ~TFrameBuffer();

private:
	std::vector<float> m_depthBuffer;
	TTexture* m_texture;
};

#endif // T_FRAMEBUFFER_H