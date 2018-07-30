#ifndef T_STRUCTS_H
#define T_STRUCTS_H

#include "glm.hpp"
#include "vec4.hpp"
#include "mat4x4.hpp"

struct TVertex {
	glm::vec4 position;
	glm::vec3 normal;
	glm::vec4 color;

	TVertex transform(const glm::mat4& proj, const glm::mat4& viewModel) const;
};

#endif // T_STRUCTS_H