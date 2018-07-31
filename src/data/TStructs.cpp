#include "TStructs.h"

TVertex TVertex::transform(const glm::mat4& mvp) const {
	TVertex tvx;
	tvx.normal = glm::normalize(glm::vec3(mvp * glm::vec4(normal, 0.0f)));
	tvx.position = mvp * position;
	tvx.color = color;
	tvx.uv = uv;
	return tvx;
}

TVertex TVertex::lerp(const TVertex& other, float amt) {
	TVertex tvx;
	tvx.normal = glm::mix(normal, other.normal, amt);
	tvx.position = glm::mix(position, other.position, amt);
	tvx.color = glm::mix(color, other.color, amt);
	tvx.uv = glm::mix(uv, other.uv, amt);
	return tvx;
}