#ifndef OBB_H
#define OBB_H

void ComputeEigenvectors(const glm::mat3& cov, glm::vec3& eigenvalues, glm::mat3& eigenvectors);

#endif