#include "TGfx.h"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <future>
#include <cstring>
#include <cmath>
#include <vector>
#include <utility>

void showError() {
	std::cerr << SDL_GetError() << std::endl;
}

TShader* GFX::g_defaultShader = new DefaultShader();

std::optional<GFX> GFX::create(const std::string title, int width, int height, float downScale) {
	downScale = downScale < 1.0f ? 1.0f : downScale;

	int tw = int(width / downScale);
	int th = int(height / downScale);

	GFX gfx;
	SDL_Init(SDL_INIT_VIDEO);

	gfx.m_window = SDL_CreateWindow(
		title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_SHOWN
	);
	if (gfx.m_window == nullptr) {
		showError();
		return {};
	}

	gfx.m_renderer = SDL_CreateRenderer(gfx.m_window, 0, SDL_RENDERER_ACCELERATED);
	if (gfx.m_renderer == nullptr) {
		showError();
		return {};
	}

	gfx.m_screenBuffer = SDL_CreateTexture(
		gfx.m_renderer,
		SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING,
		tw, th
	);

	gfx.m_shouldClose = false;
	gfx.m_windowWidth = width;
	gfx.m_windowHeight = height;
	gfx.m_drawWidth = tw;
	gfx.m_drawHeight = th;
	gfx.m_modelMatrixStack.loadIdentity();
	gfx.m_projectionMatrixStack.loadIdentity();
	
	gfx.m_defaultTarget = new TFrameBuffer(tw, th);
	gfx.m_target = nullptr;
	gfx.clear();

	gfx.m_boundTexture = nullptr;
	gfx.m_boundShader = g_defaultShader;

	const int tilesX = (tw / T_TILE_SIZE);
	const int tilesY = (th / T_TILE_SIZE);

	gfx.m_screenTiles.resize(tilesX * tilesY);

	for (int y = 0; y < tilesY; y++) {
		for (int x = 0; x < tilesX; x++) {
			gfx.m_screenTiles[x + y * tilesX] = TAABB(
				x * T_TILE_SIZE,
				y * T_TILE_SIZE,
				x * T_TILE_SIZE + T_TILE_SIZE,
				y * T_TILE_SIZE + T_TILE_SIZE
			);
		}
	}

	return std::make_optional(gfx);
}

void GFX::poll() {
	while (SDL_PollEvent(&m_event)) {
		if (m_event.type == SDL_QUIT) m_shouldClose = true;
	}
}

void GFX::flip() {
	/// Flip screen
	Uint8* pixels;
	int pitch;
	SDL_LockTexture(m_screenBuffer, nullptr, (void**) &pixels, &pitch);

	for (int y = 0; y < m_drawHeight; y++) {
		for (int x = 0; x < m_drawWidth; x++) {
			int index = x + y * m_drawWidth;
			glm::vec4 col = m_defaultTarget->texture()->get(x, y);
			int pidx = index * 3;
			int newR = int(col.r * 255);
			int newG = int(col.g * 255);
			int newB = int(col.b * 255);
			pixels[pidx + 0] = newR;
			pixels[pidx + 1] = newG;
			pixels[pidx + 2] = newB;
		}
	}

	SDL_UnlockTexture(m_screenBuffer);

	SDL_RenderClear(m_renderer);

	SDL_Rect rec = { 0, 0, m_windowWidth, m_windowHeight };
	SDL_RenderCopy(m_renderer, m_screenBuffer, nullptr, &rec);

	SDL_RenderPresent(m_renderer);
}

void GFX::destroy() {
	delete m_defaultTarget;
	SDL_DestroyTexture(m_screenBuffer);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
}

void GFX::clear(glm::vec3 color) {
	target()->clear(glm::vec4(color, 1.0f));
}

void GFX::pixel(int x, int y, glm::vec4 color) {
	target()->texture()->set(x, y, color);
}

void GFX::line(int x1, int y1, int x2, int y2, glm::vec4 color) {
	int dx = std::abs(x2 - x1);
	int sx = x1 < x2 ? 1 : -1;
	int dy = -std::abs(y2 - y1);
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;
	int e2 = 0;
	
	int x = x1;
	int y = y1;

	while (true) {
		pixel(x, y, color);

		if (x == x2 && y == y2) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
	}
}

static TVertex toScreenSpace(TVertex v, int tw, int th) {
	v.position.x = std::floor(0.5f * tw * (v.position.x + 1.0f));
	v.position.y = std::floor(0.5f * th * (v.position.y + 1.0f));
	return v;
}

static glm::vec3 barycentric(
	const glm::vec2& p,
	const glm::vec4& v0,
	const glm::vec4& v1,
	const glm::vec4& v2
) {
	glm::vec4 ab = v1 - v0;
	glm::vec4 ac = v2 - v0;
	glm::vec2 pa = glm::vec2(v0.x, v0.y) - p;

	glm::vec3 uv1 = glm::cross(glm::vec3(ac.x, ab.x, pa.x), glm::vec3(ac.y, ab.y, pa.y));

	if (std::abs(uv1.z) < 1e-2) {
		return glm::vec3(-1, 1, 1);
	}
	return (1.0f / uv1.z) * glm::vec3(uv1.z - (uv1.x + uv1.y), uv1.y, uv1.x);
}

static void clipPolygonComponent(
	std::vector<TVertex> vertices, int comp, float factor,
	std::vector<TVertex>& out)
{
	TVertex prevVert = vertices[vertices.size()-1];
	float prevComp = prevVert.position[comp] * factor;
	bool prevInside = prevComp <= prevVert.position.w;

	for (TVertex currVert : vertices) {
		float currComp = currVert.position[comp] * factor;
		bool currInside = currComp <= currVert.position.w;

		if (currInside ^ prevInside) {
			float lerpAmt = (prevVert.position.w - prevComp) /
					((prevVert.position.w - prevComp) - (currVert.position.w - currComp));
			out.push_back(prevVert.lerp(currVert, lerpAmt));
		}

		if (currInside) {
			out.push_back(currVert);
		}

		prevVert = currVert;
		prevComp = currComp;
		prevInside = currInside;
	}
}

static bool clipPolygonAxis(
	std::vector<TVertex>& vertices,
	std::vector<TVertex>& aux,
	int comp
)
{
	clipPolygonComponent(vertices, comp, 1.0f, aux);
	vertices.clear();

	if (aux.empty()) {
		return false;
	}

	clipPolygonComponent(aux, comp, -1.0f, vertices);
	aux.clear();

	return !vertices.empty();
}

static float wrap(float flt, float max) {
	if (flt > max) {
		flt -= max;
	}
	if (flt < 0.0f) {
		flt += max;
	}
	return flt;
}

std::optional<TTriangle> GFX::createTriangle(const TVertex& v0, const TVertex& v1, const TVertex& v2) {
	TTriangle tri;
	tri.v0 = v0;
	tri.v1 = v1;
	tri.v2 = v2;

	tri.vp0 = v0.position;
	tri.vp1 = v1.position;
	tri.vp2 = v2.position;

	/// Perspective Divide
	tri.v0.position /= v0.position.w;
	tri.v1.position /= v1.position.w;
	tri.v2.position /= v2.position.w;

	TVertex vt0 = tri.v0;
	TVertex vt1 = tri.v1;
	TVertex vt2 = tri.v2;

	glm::vec4 _vs1 = vt1.position - vt0.position;
	glm::vec4 _vs2 = vt2.position - vt0.position;

	/// Cull
	glm::vec3 eye = glm::vec3(modelView().matrix()[3]);
	glm::vec3 tV = glm::normalize(glm::vec3(vt0.position) - eye);
	glm::vec3 tN = glm::cross(glm::vec3(_vs1), glm::vec3(_vs2));

	if (glm::dot(tN, tV) <= 0.0f) {
		return {};
	}

	/// To screen space
	vt0 = toScreenSpace(vt0, m_drawWidth, m_drawHeight);
	vt1 = toScreenSpace(vt1, m_drawWidth, m_drawHeight);
	vt2 = toScreenSpace(vt2, m_drawWidth, m_drawHeight);

	tri.maxX = std::max(vt0.position.x, std::max(vt1.position.x, vt2.position.x));
	tri.minX = std::min(vt0.position.x, std::min(vt1.position.x, vt2.position.x));
	tri.maxY = std::max(vt0.position.y, std::max(vt1.position.y, vt2.position.y));
	tri.minY = std::min(vt0.position.y, std::min(vt1.position.y, vt2.position.y));

	tri.v0 = vt0;
	tri.v1 = vt1;
	tri.v2 = vt2;

	return std::make_optional(tri);
}

std::vector<TTile> GFX::buildTiles(const std::vector<TTriangle>& tris) {
	moodycamel::ConcurrentQueue<std::pair<int, int>> tileIDs;

	const int tilesX = (m_drawWidth / T_TILE_SIZE);
	const int tilesY = (m_drawHeight / T_TILE_SIZE);
	const glm::vec2 res(m_drawWidth, m_drawHeight);
	const glm::vec2 tilesXY(tilesX, tilesY);
	
	#pragma omp parallel for schedule(dynamic)
	for (int triangleID = 0; triangleID < tris.size(); triangleID++) {
		TTriangle tri = tris[triangleID];
		glm::vec2 triMin(tri.minX, tri.minY);
		glm::vec2 triMax(tri.maxX, tri.maxY);

		triMin /= res;
		triMax /= res;

		triMin *= tilesXY;
		triMax *= tilesXY;

		triMin = glm::floor(triMin);
		triMax = glm::ceil(triMax);

		for (int ty = triMin.y; ty < triMax.y; ty++) {
			for (int tx = triMin.x; tx < triMax.x; tx++) {
				int tileID = tx + ty * tilesX;
				tileIDs.enqueue(std::make_pair(tileID, triangleID));
			}
		}
	}

	std::vector<std::pair<int, int>> tileIDsv;
	tileIDsv.reserve(tileIDs.size_approx());

	std::pair<int, int> tileID;
	while (tileIDs.try_dequeue(tileID)) {
		tileIDsv.push_back(tileID);
	}

	std::sort(tileIDsv.begin(), tileIDsv.end(), [&](std::pair<int, int> a, std::pair<int, int> b){
		return a.first < b.first;
	});

	std::vector<TTile> tiles;
	int tid = -1;
	for (auto ttid : tileIDsv) {
		if (ttid.first != tid) {
			tiles.push_back(TTile());
			TTile& tile = tiles.back();
			tile.x = (ttid.first % tilesX) * T_TILE_SIZE;
			tile.y = (ttid.first / tilesX) * T_TILE_SIZE;
			tid = ttid.first;
		}
		TTile& tile = tiles.back();
		tile.triangles.push_back(tris[ttid.second]);
	}
	
	// std::cout << "TILES THIS FRAME: " << tiles.size() << std::endl;
	return tiles;
}

void GFX::drawTile(const TTile& tile) {
	const float BX = 1.0f / m_drawWidth;
	const float BY = 1.0f / m_drawHeight;

	for (TTriangle tri : tile.triangles) {
		for (int y = tile.y; y < tile.y + T_TILE_SIZE; y++) {
			for (int x = tile.x; x < tile.x + T_TILE_SIZE; x++) {
				glm::vec3 bc = barycentric(
					glm::vec2(x, y),
					tri.v0.position,
					tri.v1.position,
					tri.v2.position
				);

				if (bc.x < -BX || bc.y < -BY || bc.z < 0.0f) { continue; }

				glm::vec3 P = glm::vec3(
					bc.x / tri.vp0.w,
					bc.y / tri.vp1.w,
					bc.z / tri.vp2.w
				);
				float d = (P.x + P.y + P.z);
				P = (1.0f / d) * P;

				float z = d / 3.0f;

				if (target()->depth(x, y) < z) {
					glm::vec4 col = P.x * tri.v0.color + P.y * tri.v1.color + P.z * tri.v2.color;
					glm::vec2 uv = P.x * tri.v0.uv + P.y * tri.v1.uv + P.z * tri.v2.uv;
					uv.x = wrap(uv.x, 1.0f);
					uv.y = wrap(uv.y, 1.0f);
					
					TPixelInput pi;
					pi.boundTexture = m_boundTexture;
					pi.vertexPositions = P.x * tri.vp0 + P.y * tri.vp1 + P.z * tri.vp2;
					pi.normals = glm::normalize(P.x * tri.v0.normal + P.y * tri.v1.normal + P.z * tri.v2.normal);
					pi.texCoords = uv;
					pi.vertexColors = col;

					glm::vec4 pixelColor = glm::clamp(boundShader()->pixel(pi), 0.0f, 1.0f);
					if (!boundShader()->m_discard) {
						pixel(x, y, pixelColor);
						target()->depth(x, y, z);
					} else {
						boundShader()->m_discard = false;
					}
				}
			}
		}
	}

	// line(tile.x, tile.y, tile.x+T_TILE_SIZE, tile.y, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	// line(tile.x+T_TILE_SIZE, tile.y, tile.x+T_TILE_SIZE, tile.y+T_TILE_SIZE, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	// line(tile.x, tile.y+T_TILE_SIZE, tile.x+T_TILE_SIZE, tile.y+T_TILE_SIZE, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	// line(tile.x, tile.y, tile.x, tile.y+T_TILE_SIZE, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

std::vector<TVertex> GFX::triangleProcess(const TVertex& v0, const TVertex& v1, const TVertex& v2) {
	glm::mat4 mvp = projection().matrix() * modelView().matrix();

	std::vector<TVertex> vertices, aux;
	vertices.insert(vertices.end(), {
		boundShader()->vertex(projection().matrix(), modelView().matrix(), v0),
		boundShader()->vertex(projection().matrix(), modelView().matrix(), v1),
		boundShader()->vertex(projection().matrix(), modelView().matrix(), v2)
	});
	
	std::vector<TVertex> triangles;
	
	if (clipPolygonAxis(vertices, aux, 0) &&
		clipPolygonAxis(vertices, aux, 1) &&
		clipPolygonAxis(vertices, aux, 2))
	{
		TVertex initial = vertices[0];
		for (int i = 1; i < vertices.size() - 1; i++) {
			TTriangle tri;
			triangles.push_back(initial);
			triangles.push_back(vertices[i]);
			triangles.push_back(vertices[i+1]);
		}
	}
	return triangles;
}

void GFX::mesh(const std::vector<TVertex>& vertices, const std::vector<int>& indices) {
	moodycamel::ConcurrentQueue<TTriangle> triangles;

	auto clk = BEGIN_BENCH;

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < indices.size(); i+=3) {
		TVertex v0 = vertices[indices[i + 0]];
		TVertex v1 = vertices[indices[i + 1]];
		TVertex v2 = vertices[indices[i + 2]];

		std::vector<TVertex> verticesProc = triangleProcess(v0, v1, v2);
		for (int i = 0; i < verticesProc.size(); i+=3) {
			TVertex vt0 = verticesProc[i];
			TVertex vt1 = verticesProc[i+1];
			TVertex vt2 = verticesProc[i+2];

			std::optional<TTriangle> optTri = createTriangle(vt0, vt1, vt2);
			if (optTri.has_value()) {
				triangles.enqueue(optTri.value());
			}
		}
	}
	END_BENCH(clk, "CLIPPING and TRANSFORMATIONS");

	std::vector<TTriangle> trianglesVec;
	trianglesVec.reserve(triangles.size_approx());
	
	TTriangle tri;
	while (triangles.try_dequeue(tri)) {
		trianglesVec.push_back(tri);
	}

	clk = BEGIN_BENCH;
	std::vector<TTile> tiles = buildTiles(trianglesVec);
	END_BENCH(clk, "TILE GENERATION");

	clk = BEGIN_BENCH;
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < tiles.size(); i++) {
		TTile tile = tiles[i];
		drawTile(tile);
	}
	END_BENCH(clk, "RENDERING");
}