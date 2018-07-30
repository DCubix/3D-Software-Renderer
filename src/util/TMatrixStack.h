#ifndef T_MATRIX_STACK_H
#define T_MATRIX_STACK_H

#include <array>

#include "vec3.hpp"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp"

#define T_MAX_MATRIX_TACK_DEPTH 128

class TMatrixStack {
public:
	TMatrixStack() : m_stackPointer(0) {}

	void loadIdentity();
	void loadMatrix(const glm::mat4& m);
	void multMatrix(const glm::mat4& m);
	void pushMatrix();
	void popMatrix();

	void perspective(float fov, float aspect, float znear, float zfar);

	void translate(const glm::vec3& t);
	void rotate(float angle, const glm::vec3& axis);
	void scale(const glm::vec3& s);

	glm::mat4 matrix() const { return m_matrixStack[m_stackPointer]; }

private:
	std::array<glm::mat4, T_MAX_MATRIX_TACK_DEPTH> m_matrixStack;
	int m_stackPointer;
};

#endif // T_MATRIX_STACK_H