#include <iostream>
#include <vector>

#include "util/TGfx.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class LightShader : public DefaultShader {
public:
	TTexture* matcap;
	glm::vec3 L = glm::vec3(-1.0f);

	glm::vec4 pixel(TPixelInput in) override {
		glm::vec3 V = glm::normalize(glm::vec3(-in.vertexPositions));

		glm::vec4 supColor = DefaultShader::pixel(in);
		float nl = glm::min(glm::max(glm::dot(in.normals, L), 0.0f) + 0.15f, 1.0f);
		
		glm::vec3 r = glm::reflect(V, in.normals);
		float m = 2.0f * glm::sqrt(
			glm::pow(r.x, 2.0f) +
			glm::pow(r.y, 2.0f) +
			glm::pow(r.z + 1.0f, 2.0f)
		);
		glm::vec2 texCo = glm::vec2(r) / m + 0.5f;

		glm::vec4 matCapColor = matcap->getBilinear(texCo.x, texCo.y);

		return glm::vec4(glm::vec3(supColor * matCapColor * nl), 1.0f);
		// return glm::vec4(in.normals * 0.5f + 0.5f, 1.0f);
	}
};

const int TRENDER_WIDTH = 640;
const int TRENDER_HEIGHT = 480;
const float TRENDER_DOWNSCALE = 1.0f;

int main(int argc, char** argv) {
	float rot = 0.0f;

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

	TTexture* tex = new TTexture("tex.jpg");
	TTexture* matcap = new TTexture("matcap4.jpg");
	LightShader* shd = new LightShader();

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

			std::cout << "FPS: " << (1.0f/delta) << std::endl;
		}

		float pos = (std::sin(rot * 1.5f) * 0.5f + 0.5f) * 8.0f;
		if (canRender) {
			gfx.clear();

			gfx.projection().loadIdentity();
			gfx.projection().perspective(glm::radians(70.0f), 640.0f/480.0f, 0.01f, 200.0f);

			gfx.modelView().loadIdentity();
			gfx.modelView().translate(glm::vec3(0.0f, 0.2f, -5));
			gfx.modelView().rotate(M_PI-M_PI/4, glm::vec3(1, 0, 0));
			gfx.modelView().rotate(rot, glm::vec3(0, 1, 0));
			//gfx.modelView().rotate(rot*1.25f, glm::vec3(0, 0, 1));

			// gfx.boundTexture(tex);

			shd->matcap = matcap;
			gfx.boundShader(shd);
			gfx.mesh(vertices, indices);

			gfx.flip();
		}
	}

	delete tex;
	delete matcap;
	delete shd;

	gfx.destroy();
	return 0;
}
