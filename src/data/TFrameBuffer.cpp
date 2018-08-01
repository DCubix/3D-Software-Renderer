#include "TFrameBuffer.h"

TFrameBuffer::TFrameBuffer(int w, int h) {
	m_texture = new TTexture(w, h);
	m_depthBuffer.resize(w * h);
	std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), 0.0f);
}

float TFrameBuffer::depth(int x, int y) const {
	if (x < 0 || x >= width() || y < 0 || y >= height()) {
		return 0.0f;
	}
	return m_depthBuffer[x + y * width()];
}

void TFrameBuffer::depth(int x, int y, float d) {
	if (x < 0 || x >= width() || y < 0 || y >= height()) {
		return;
	}
	m_depthBuffer[x + y * width()] = d;
}

TFrameBuffer::~TFrameBuffer() {
	delete m_texture;
}

void TFrameBuffer::clear(const glm::vec4& color) {
	std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), 0.0f);
	m_texture->clear(color);
}