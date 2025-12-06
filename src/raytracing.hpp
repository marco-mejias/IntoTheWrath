#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cfloat>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace glm;

// ============================================================================
// VARIABLES
// ============================================================================
float mousePosX;
float mousePosY;

glm::vec3 camera_position(0.0f);
glm::vec3 ray_direction(0.0f);
bool draw_ray = true;

std::vector<glm::vec3> ray_trail;

// ============================================================================
// STRUCTS
// ============================================================================

//Rajos temporals

struct PersistentRay {
	glm::vec3 origin;
	glm::vec3 dir;
	float length;
	float timeLeft;
};

glm::vec3 ray_end;
std::vector<PersistentRay> g_PersistentRays;
double g_LastRayTime = -5.0;
double g_LastRayDuplicateTime = 0.0;

const float RAY_LIFETIME = 8.0f; //Segons que dura un ray persistent
const float RAY_DUPLICATE_INTERVAL = 5.0f; //Cada cuant generem un ray

// ============================================================================
// FUNCIONES
// ============================================================================

//Calculem AABB del Model
AABB GetAABBFromModelMatrix(const glm::mat4& M) {

	const glm::vec3 vertices[8] = {
		{-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f},
		{-0.5f, 0.5f,-0.5f}, {0.5f, 0.5f,-0.5f},
		{-0.5f,-0.5f, 0.5f}, {0.5f, 0.5f, 0.5f},
		{0.5f,-0.5f, 0.5f},  {0.5f, 0.5f, 0.5f}
	};

	glm::vec3 min(FLT_MAX), max(-FLT_MAX);

	for (auto& v : vertices) {
		glm::vec3 world = glm::vec3(M * glm::vec4(v, 1.0f));
		min = glm::min(min, world);
		max = glm::max(max, world);
	}
	return { min, max };
}

// Comprobem si raig intersecta amb alguna ABB
bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir,	const AABB& box, float& outDist) {

	float tmin = (box.min.x - rayOrigin.x) / rayDir.x;
	float tmax = (box.max.x - rayOrigin.x) / rayDir.x;
	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (box.min.y - rayOrigin.y) / rayDir.y;
	float tymax = (box.max.y - rayOrigin.y) / rayDir.y;
	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax)) return false;

	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	float tzmin = (box.min.z - rayOrigin.z) / rayDir.z;
	float tzmax = (box.max.z - rayOrigin.z) / rayDir.z;
	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax)) return false;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	outDist = tmin > 0 ? tmin : tmax;
	return outDist > 0;
}

// ============================================================================
// DISPARO DEL RAY
// ============================================================================
void ShootFPVRay(float g_FPVYaw, float g_FPVPitch, glm::vec3 g_FPVPos) {
	double now = glfwGetTime();
	if (now - g_LastRayTime < 5.0) return; // Cooldown de 5 segons
	g_LastRayTime = now;

	// Dirección del rayo desde la cámara amb Yaw i Pitch
	float cy = cos(glm::radians(g_FPVYaw));
	float sy = sin(glm::radians(g_FPVYaw));
	float cp = cos(glm::radians(g_FPVPitch));
	float sp = sin(glm::radians(g_FPVPitch));
	glm::vec3 rayDir = glm::normalize(glm::vec3(cy * cp, sp, sy * cp));

	float closestDist = 100.0f; //Distancia maxima del raig
	int hitIndex = -1;
	glm::vec3 hitPoint;

	// Comproba colisions amb els props
	for (size_t i = 0; i < g_Props.size(); ++i) {
		float hitDist = 0.0f;
		if (RayIntersectsAABB(g_FPVPos, rayDir, g_Props[i].hitbox, hitDist)) {
			if (hitDist < closestDist) {
				closestDist = hitDist;
				hitIndex = (int)i;
				hitPoint = g_FPVPos + rayDir * hitDist;
			}
		}
	}

	glm::vec3 rayEnd = g_FPVPos + rayDir * closestDist;

	// Creem raig persistent
	PersistentRay newRay;
	newRay.origin = g_FPVPos;
	newRay.dir = rayDir;
	newRay.length = closestDist;
	newRay.timeLeft = RAY_LIFETIME; // p.ej. 8 s

	g_PersistentRays.push_back(newRay);

	if (hitIndex != -1)
		fprintf(stderr, "[RAY HIT] prop=%d impact=(%.2f, %.2f, %.2f) dist=%.2f\n",
			hitIndex, hitPoint.x, hitPoint.y, hitPoint.z, closestDist);
}

//Actualitzem temps de vida dels rays persistents

void UpdatePersistentRays(float dt) 
{
	for (auto& r : g_PersistentRays)
		r.timeLeft -= dt;

	// Iterador element que volem eliminar
	auto it = std::remove_if(
		g_PersistentRays.begin(),
		g_PersistentRays.end(),
		[](const PersistentRay& ray) {
			// Si no li queda temps, eliminem
			return ray.timeLeft <= 0.0f;
		}
	);

	// Erase del iteradot
	g_PersistentRays.erase(it, g_PersistentRays.end());

}


// ============================================================================
// ACTUALIZAR RAYOS (cada frame)
// ============================================================================
void UpdateRays(float dt, float g_FPVYaw, float g_FPVPitch, glm::vec3 g_FPVPos) {
	double now = glfwGetTime();

	//Calcula direccio camera
	glm::vec3 rayOrigin = g_FPVPos;
	glm::vec3 rayDir = glm::normalize(glm::vec3(
		cosf(glm::radians(g_FPVYaw)) * cosf(glm::radians(g_FPVPitch)),
		sinf(glm::radians(g_FPVPitch)),
		sinf(glm::radians(g_FPVYaw)) * cosf(glm::radians(g_FPVPitch))
	));

	float maxDistance = 10.0f;
	float hitDistance = maxDistance;
	bool hitDetected = false;

	//Detecta colisions --> Cambiar en un futur si es massa cost computacional

	for (auto& p : g_Props) {
		float dist;
		if (RayIntersectsAABB(rayOrigin, rayDir, p.hitbox, dist)) {
			if (dist < hitDistance) {
				hitDetected = true;
				hitDistance = dist;
			}
		}
	}

	glm::vec3 rayEnd = rayOrigin + rayDir * hitDistance;
	ray_trail.push_back(rayEnd);
	if (ray_trail.size() > 100)
		ray_trail.erase(ray_trail.begin());

	if (hitDetected)
		fprintf(stderr, "[DEBUG] Rayo impactó a %.2f m (%.2f, %.2f, %.2f)\n",
			hitDistance, rayEnd.x, rayEnd.y, rayEnd.z);

	
}

// ============================================================================
// Dibuixa AABB
// ============================================================================
void DrawAABBWireframe(const AABB& b) {
	glm::vec3 v[8] = {
		{b.min.x, b.min.y, b.min.z}, {b.max.x, b.min.y, b.min.z},
		{b.max.x, b.max.y, b.min.z}, {b.min.x, b.max.y, b.min.z},
		{b.min.x, b.min.y, b.max.z}, {b.max.x, b.min.y, b.max.z},
		{b.max.x, b.max.y, b.max.z}, {b.min.x, b.max.y, b.max.z}
	};

	glBegin(GL_LINES);
	int edges[12][2] = {
		{0,1},{1,2},{2,3},{3,0},
		{4,5},{5,6},{6,7},{7,4},
		{0,4},{1,5},{2,6},{3,7}
	};
	for (auto& e : edges) {
		glVertex3fv(glm::value_ptr(v[e[0]]));
		glVertex3fv(glm::value_ptr(v[e[1]]));
	}
	glEnd();
}

// ============================================================================
// RAYCAST NORMAL 
// ============================================================================
void RaycastFPV(float g_FPVYaw, float g_FPVPitch, glm::vec3 g_FPVPos)
{
	// Direcció cam
	float cy = cosf(glm::radians(g_FPVYaw));
	float sy = sinf(glm::radians(g_FPVYaw));
	float cp = cosf(glm::radians(g_FPVPitch));
	float sp = sinf(glm::radians(g_FPVPitch));

	//ACTUALITZEM VARIABLES GLOBALS DEL RAY

	glm::vec3 front = glm::normalize(glm::vec3(cy * cp, sp, sy * cp));

	camera_position = g_FPVPos;
	ray_direction = front;  

	// Raig principal
	glm::vec3 ray_origin = camera_position;
	glm::vec3 ray_dir = glm::normalize(ray_direction);
	float maxDist = 10.0f;
	float closestDist = maxDist;
	int hitIndex = -1;

	// Detección de colisiones con props
	for (size_t i = 0; i < g_Props.size(); ++i) {
		float hitDist = 0.0f;
		if (RayIntersectsAABB(ray_origin, ray_dir, g_Props[i].hitbox, hitDist)) {
			if (hitDist < closestDist) {
				closestDist = hitDist;
				hitIndex = (int)i;
			}
		}
	}

	if (hitIndex != -1) {
		ray_end = ray_origin + ray_dir * closestDist;
		fprintf(stderr, "[RAY HIT] colisión con prop=%d dist=%.2f pos=(%.2f, %.2f, %.2f)\n",
			hitIndex, closestDist, ray_end.x, ray_end.y, ray_end.z);
	}
	else {
		ray_end = ray_origin + ray_dir * maxDist;
	}

}


// ============================================================================
// Dibuixar ray normal i persistents
// ============================================================================
void DibuixaRay()
{
	if (!draw_ray) return;

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(ProjectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(ViewMatrix));

	// Dibuix AABB dels props
	glLineWidth(1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	for (const auto& p : g_Props)
		DrawAABBWireframe(p.hitbox);

	glEnable(GL_DEPTH_TEST);

	//Trail del ray
	ray_trail.push_back(ray_end);
	if (ray_trail.size() > 100)
		ray_trail.erase(ray_trail.begin());

	glLineWidth(3.0f);
	glBegin(GL_LINE_STRIP);
	for (size_t i = 0; i < ray_trail.size(); ++i) {
		float alpha = 1.0f - (float)i / ray_trail.size();
		glColor4f(1.0f, alpha, 0.0f, alpha);
		glVertex3f(ray_trail[i].x, ray_trail[i].y, ray_trail[i].z);
	}
	glEnd();
}


//dibuix persintants rays

void DrawPersistentRays() 
{
	if (g_PersistentRays.empty()) return;

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(ProjectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(ViewMatrix));

	glLineWidth(3.0f);
	glBegin(GL_LINES);

	for (auto& r : g_PersistentRays) {
		float t = r.timeLeft / RAY_LIFETIME;
		glColor3f(1.0f, t * 0.5f, 0.0f); // vermell cap a tronja
		glm::vec3 end = r.origin + r.dir * r.length;
		glVertex3fv(glm::value_ptr(r.origin));
		glVertex3fv(glm::value_ptr(end));
	}

	glEnd();

	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW); glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}


