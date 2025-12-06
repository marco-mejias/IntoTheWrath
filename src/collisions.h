#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "objLoader.h"

OBB PropToOBB(const Prop& p);

float ClosestPtSegmentOBB_Analytic(
	const glm::vec3& a,
	const glm::vec3& b,
	const glm::vec3& e,
	glm::vec3& outSeg,
	glm::vec3& outBox);

bool ResolveCapsuleOBBSlidingCollision(Capsule& cap, const OBB& box);

void CheckPlayerSlidingCollisionNew(glm::vec3 nextPos, const float radius, glm::vec3& currentPos, float eyesHeight, std::vector<COBJModel*> vHitboxOBJ);

#endif