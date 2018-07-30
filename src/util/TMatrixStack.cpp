#include "TMatrixStack.h"

void TMatrixStack::loadIdentity() {
	m_matrixStack[m_stackPointer] = glm::mat4(1.0f);
}

void TMatrixStack::loadMatrix(const glm::mat4& m) {
	m_matrixStack[m_stackPointer] = m;
}

void TMatrixStack::multMatrix(const glm::mat4& m) {
	glm::mat4 tmp = m_matrixStack[m_stackPointer];
	m_matrixStack[m_stackPointer] = tmp * m;
}

void TMatrixStack::pushMatrix() {
	if (m_stackPointer < (T_MAX_MATRIX_TACK_DEPTH - 1)) {
		m_stackPointer++;
		m_matrixStack[m_stackPointer] = m_matrixStack[m_stackPointer - 1];
	}
}

void TMatrixStack::popMatrix() {
	if (m_stackPointer > 0)
		m_stackPointer--;
}

void TMatrixStack::translate(const glm::vec3& t) {
	glm::mat4 tmp = m_matrixStack[m_stackPointer],
			  trl = glm::translate(glm::mat4(1.0f), t);
	m_matrixStack[m_stackPointer] = tmp * trl;
}

void TMatrixStack::rotate(float angle, const glm::vec3& axis) {
	glm::mat4 tmp = m_matrixStack[m_stackPointer],
			  rot = glm::rotate(glm::mat4(1.0f), angle, axis);
	m_matrixStack[m_stackPointer] = tmp * rot;
}

void TMatrixStack::scale(const glm::vec3& s) {
	glm::mat4 tmp = m_matrixStack[m_stackPointer],
			  scl = glm::scale(glm::mat4(1), s);
	m_matrixStack[m_stackPointer] = tmp * scl;
}

void TMatrixStack::perspective(float fov, float aspect, float znear, float zfar) {
	glm::mat4 tmp = m_matrixStack[m_stackPointer],
			  pmt = glm::perspective(fov, aspect, znear, zfar);
	m_matrixStack[m_stackPointer] = tmp * pmt;
}