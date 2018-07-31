#include <iostream>
#include <vector>

#include "util/TGfx.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class LightShader : public DefaultShader {
public:
	glm::vec3 L = glm::vec3(-1.0f);

	glm::vec4 pixel(TPixelInput in) override {
		glm::vec3 V = glm::normalize(glm::vec3(-in.vertexPositions));
		glm::vec3 H = glm::normalize(L + V);

		glm::vec4 supColor = DefaultShader::pixel(in);
		float nl = glm::max(glm::dot(in.normals, L), 0.0f) + 0.2f;
		float spec = glm::pow(glm::max(0.0f, glm::dot(in.normals, H)), 60.0f) * nl;

		return glm::vec4(glm::vec3(supColor * (nl + spec)), 1.0f);
		// return glm::vec4(in.normals * 0.5f + 0.5f, 1.0f);
	}
};

const int TRENDER_WIDTH = 640;
const int TRENDER_HEIGHT = 480;
const float TRENDER_DOWNSCALE = 1.0f;

int main(int argc, char** argv) {
	float rot = 0.0f;

	// const TVertex CUBE_V[] = {
	// 	{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ {  1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ {  1.0f,  1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ { -1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },

	// 	{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ {  1.0f, -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ {  1.0f,  1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// 	{ { -1.0f,  1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }
	// };
	// const int CUBE_I[] = {
	// 	0, 1, 2,  // FRONT
	// 	2, 3, 0,
	// 	5, 4, 7,  // BACK
	// 	7, 6, 5,
	// 	1, 5, 6,  // RIGHT
	// 	6, 2, 1,
	// 	4, 0, 3,  // LEFT
	// 	3, 7, 4,
	// 	3, 2, 6,  // TOP
	// 	6, 7, 3,
	// 	1, 0, 4,  // BOTTOM
	// 	4, 5, 1
	// };

	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile("teapot.obj",
			aiPostProcessSteps::aiProcess_Triangulate |
			aiPostProcessSteps::aiProcess_FlipUVs
	);

	std::vector<TVertex> vertices;
	std::vector<int> indices;

	if (scene != nullptr) {
		const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
		const aiColor4D aiOneVector4(1.0f, 1.0f, 1.0f, 1.0f);

		for (int m = 0; m < scene->mNumMeshes; m++) {
			aiMesh* mesh = scene->mMeshes[m];
			bool hasPositions = mesh->HasPositions();
			bool hasNormals = mesh->HasNormals();
			bool hasUVs = mesh->HasTextureCoords(0);
			bool hasTangents = mesh->HasTangentsAndBitangents();
			bool hasColors = mesh->HasVertexColors(0);

			for (int i = 0; i < mesh->mNumVertices; i++) {
				TVertex v;
				const aiVector3D pos = hasPositions ? mesh->mVertices[i] : aiZeroVector;
				const aiVector3D normal = hasNormals ? mesh->mNormals[i] : aiZeroVector;
				const aiVector3D texCoord = hasUVs ? mesh->mTextureCoords[0][i] : aiZeroVector;
				// const aiVector3D tangent = hasTangents ? mesh->mTangents[i] : aiZeroVector;
				const aiColor4D color = hasColors ? mesh->mColors[0][i] : aiOneVector4;

				v.position = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
				v.normal = glm::vec3(normal.x, normal.y, normal.z);
				v.uv = glm::vec2(texCoord.x, texCoord.y);
				v.color = glm::vec4(color.r, color.g, color.b, color.a);

				vertices.push_back(v);
			}

			for (int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
				}
			}
		}
	}

	GFX gfx = GFX::create("TRender", TRENDER_WIDTH, TRENDER_HEIGHT, TRENDER_DOWNSCALE).value();

	TTexture* tex = new TTexture("tex_low.jpg");
	TShader* shd = new LightShader();

	const double timeStep = 1.0 / 60.0;
	double lastTime = gfx.time();
	double accum = 0.0;

	bool canRender = false;
	while (!gfx.shouldClose()) {
		canRender = false;

		double currTime = gfx.time();
		double delta = currTime - lastTime;
		lastTime = currTime;
		accum += delta;

		gfx.poll();

		while (accum >= timeStep) {
			accum -= timeStep;
			rot += 0.01f;
			canRender = true;
		}

		float pos = (std::sin(rot * 1.5f) * 0.5f + 0.5f) * 6.0f;
		if (canRender) {
			gfx.clear();

			gfx.projection().loadIdentity();
			gfx.projection().perspective(glm::radians(60.0f), 640.0f/480.0f, 0.01f, 200.0f);

			gfx.modelView().loadIdentity();
			gfx.modelView().translate(glm::vec3(0.0f, 1.0f, -8.0f));
			gfx.modelView().rotate(M_PI, glm::vec3(1, 0, 0));
			gfx.modelView().rotate(rot, glm::vec3(0, 1, 0));
			//gfx.modelView().rotate(rot*1.25f, glm::vec3(0, 0, 1));

			gfx.boundTexture(tex);
			gfx.boundShader(shd);
			for (int i = 0; i < indices.size(); i+=3) {
				TVertex v0 = vertices[indices[i + 0]];
				TVertex v1 = vertices[indices[i + 1]];
				TVertex v2 = vertices[indices[i + 2]];
				gfx.triangle(v0, v1, v2);
			}

			gfx.flip();
		}
	}

	delete tex;
	delete shd;

	gfx.destroy();
	return 0;
}
