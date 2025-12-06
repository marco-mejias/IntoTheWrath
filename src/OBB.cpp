#include "stdafx.h"
#include "OBB.h"

void ComputeEigenvectors(const glm::mat3& cov, glm::vec3& eigenvalues, glm::mat3& eigenvectors)
{
	// Simple Jacobi iterative method for 3x3 symmetric matrix
	const int MAX_ITERATIONS = 50;
	const float EPSILON = 1e-8f;

	eigenvectors = glm::mat3(1.0f);  // Start with identity matrix
	glm::mat3 A = cov;

	for (int iter = 0; iter < MAX_ITERATIONS; ++iter)
	{
		// Find largest off-diagonal element
		int p = 0, q = 1;
		float maxOffDiag = std::abs(A[0][1]);
		if (std::abs(A[0][2]) > maxOffDiag)
		{
			p = 0; q = 2;
			maxOffDiag = std::abs(A[0][2]);
		}
		if (std::abs(A[1][2]) > maxOffDiag)
		{
			p = 1; q = 2;
			maxOffDiag = std::abs(A[1][2]);
		}

		if (maxOffDiag < EPSILON) break;

		// Compute Jacobi rotation
		float theta = 0.5f * atan2(2.0f * A[p][q], A[q][q] - A[p][p]);
		float c = cos(theta);
		float s = sin(theta);

		// Update matrix A
		float App = A[p][p];
		float Aqq = A[q][q];
		float Apq = A[p][q];

		A[p][p] = c * c * App - 2 * c * s * Apq + s * s * Aqq;
		A[q][q] = s * s * App + 2 * c * s * Apq + c * c * Aqq;
		A[p][q] = A[q][p] = 0.0f;

		for (int k = 0; k < 3; ++k)
		{
			if (k != p && k != q)
			{
				float Akp = A[k][p];
				float Akq = A[k][q];
				A[k][p] = A[p][k] = c * Akp - s * Akq;
				A[k][q] = A[q][k] = s * Akp + c * Akq;
			}
		}

		// Update eigenvectors
		for (int k = 0; k < 3; ++k)
		{
			float Vkp = eigenvectors[k][p];
			float Vkq = eigenvectors[k][q];
			eigenvectors[k][p] = c * Vkp - s * Vkq;
			eigenvectors[k][q] = s * Vkp + c * Vkq;
		}
	}

	eigenvalues = glm::vec3(A[0][0], A[1][1], A[2][2]);
}

