#ifndef T_STRUCTS_H
#define T_STRUCTS_H

#include "glm.hpp"
#include "vec4.hpp"
#include "mat4x4.hpp"

#include "TTexture.h"

#include <array>

struct TVertex {
	glm::vec4 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec4 color;

	TVertex transform(const glm::mat4& mvp) const;
	TVertex lerp(const TVertex& other, float amt);
};

struct TTriangle {
	TVertex v0, v1, v2;
};

struct TPixelInput {
	glm::vec4 vertexPositions;
	glm::vec4 vertexColors;
	glm::vec3 normals;
	glm::vec2 texCoords;
	TTexture* boundTexture;
};

class TShader {
	friend class GFX;
public:
	virtual TVertex vertex(glm::mat4 projection, glm::mat4 viewModel, TVertex vertex) = 0;
	virtual glm::vec4 pixel(TPixelInput input) = 0;

	glm::vec4 discard() { m_discard = true; return glm::vec4(0.0f); }
protected:
	bool m_discard;
};

class DefaultShader : public TShader {
public:
	TVertex vertex(glm::mat4 projection, glm::mat4 viewModel, TVertex in) override {
		return in.transform(projection * viewModel);
	}

	glm::vec4 pixel(TPixelInput in) override {
		glm::vec4 texCol = in.boundTexture != nullptr ?
						in.boundTexture->getBilinear(in.texCoords.x, in.texCoords.y) :
						glm::vec4(1.0f);
		return texCol;
	}
};

#endif // T_STRUCTS_H