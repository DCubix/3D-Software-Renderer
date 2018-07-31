#include "TFrameBuffer.h"

TFrameBuffer::TFrameBuffer(int w, int h) {
	m_texture = new TTexture(w, h);
	m_depthBuffer = new float[w * h];
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

	if (m_depthBuffer != nullptr) {
		delete[] m_depthBuffer;
		m_depthBuffer = nullptr;
	}
}

void TFrameBuffer::clear(const glm::vec4& color) {
	int pixelCount = width() * height();
	std::fill_n(m_depthBuffer, pixelCount, 0.0f);
	std::fill_n(m_texture->pixels(), pixelCount, color);
}