#include <iostream>

#include "util/TGfx.h"

const int TRENDER_WIDTH = 640;
const int TRENDER_HEIGHT = 480;
const float TRENDER_DOWNSCALE = 1.0f;

#define LEN(x) (sizeof(x) / sizeof(x[0]))

int main(int argc, char** argv) {
	float rot = 0.0f;

	const TVertex CUBE_V[] = {
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.2f, 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.2f, 0.0f, 1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.2f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.2f, 0.0f, 0.2f, 1.0f } },

		{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ {  1.0f,  1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } }
	};
	const int CUBE_I[] = {
		0, 1, 2,  // FRONT
		2, 3, 0,
		5, 4, 7,  // BACK
		7, 6, 5,
		1, 5, 6,  // RIGHT
		6, 2, 1,
		4, 0, 3,  // LEFT
		3, 7, 4,
		3, 2, 6,  // TOP
		6, 7, 3,
		1, 0, 4,  // BOTTOM
		4, 5, 1
	};

	GFX gfx = GFX::create("TRender", TRENDER_WIDTH, TRENDER_HEIGHT, TRENDER_DOWNSCALE).value();
	while (!gfx.shouldClose()) {
		rot += 0.01f;

		gfx.clear();

		gfx.projection().loadIdentity();
		gfx.projection().perspective(glm::radians(60.0f), 640.0f/480.0f, 0.01f, 200.0f);

		gfx.modelView().loadIdentity();
		gfx.modelView().translate(glm::vec3(0.0f, 0.0f, -5.0f));
		gfx.modelView().rotate(rot, glm::vec3(0, 1, 0));
		gfx.modelView().rotate(rot*1.25f, glm::vec3(0, 0, 1));

		for (int i = 0; i < LEN(CUBE_I); i+=3) {
			TVertex v0 = CUBE_V[CUBE_I[i + 0]];
			TVertex v1 = CUBE_V[CUBE_I[i + 1]];
			TVertex v2 = CUBE_V[CUBE_I[i + 2]];
			gfx.triangle(v0, v1, v2);
		}

		gfx.update();
	}
	gfx.destroy();
	return 0;
}
