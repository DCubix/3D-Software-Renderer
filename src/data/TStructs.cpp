#include "TStructs.h"

TVertex TVertex::transform(const glm::mat4& proj, const glm::mat4& viewModel) const {
	glm::mat4 mvp = proj * viewModel;
	TVertex tvx;
	tvx.normal = glm::normalize(glm::vec3(mvp * glm::vec4(normal, 0.0f)));
	tvx.position = mvp * position;
	tvx.color = color;
	return tvx;
}