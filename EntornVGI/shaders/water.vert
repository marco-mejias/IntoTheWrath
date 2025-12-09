#version 460 core

layout(location = 0) in vec3 in_Vertex;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

uniform float uTime;   // glfwGetTime()

out vec3 vWorldPos;
out vec3 vNormal;
out float vWave;       // altura relativa de l’ona en aquest punt

// --- Paràmetres del mar ---
const float WATER_BASE = -2.0;  // nivell mitjà del mar (per sota del vaixell)
const float DERIV_EPS  = 1.0;   // pas per calcular derivades (normal)

// Funció que genera l'alçada de les onades en una posició del pla
float oceanWaves(vec2 p, float t)
{
    // Tres ones en direccions diferents (swell + xop-xop)
    vec2 d1 = normalize(vec2( 1.0,  0.3));  // ona llarga
    vec2 d2 = normalize(vec2(-0.4,  1.0));  // mitjana
    vec2 d3 = normalize(vec2( 0.7, -0.6));  // més trencada

    // Frequències baixes ⇒ onades grans visibles des de lluny
    float w1 = sin(dot(p, d1) * 0.035 + t * 0.20);  // swell llarg
    float w2 = sin(dot(p, d2) * 0.070 + t * 0.45);  // ona mitjana
    float w3 = sin(dot(p, d3) * 0.140 - t * 0.85);  // detall

    // Combinació amb pesos generosos (aquí ja fem “señor mar”)
    float h = w1 * 0.9 + w2 * 0.6 + w3 * 0.3;

    // Escala global d’amplitud (ajusta si es passa o es queda curt)
    return h * 0.9;   // valor típic ≈ [-0.9, 0.9]
}

void main()
{
    float t = uTime * 0.8;

    // Partim d’un pla gran en XZ (ignorem la Y original)
    vec3 pos = in_Vertex;
    vec2 p   = pos.xz;

    // Alçada “relativa” de l’ona
    float h = oceanWaves(p, t);

    // Mar oscillant al voltant de WATER_BASE, però sempre per sota de 0
    // Aproximadament: [-2.9, -1.1]
    pos.y = WATER_BASE + h;

    // Guardem l'altura de l'ona (relativa) per al fragment shader
    vWave = h;

    // --- Normal: derivades numèriques en X i Z ---
    float hX = oceanWaves(p + vec2(DERIV_EPS, 0.0), t);
    float hZ = oceanWaves(p + vec2(0.0, DERIV_EPS), t);

    vec3 dX = vec3(DERIV_EPS, hX - h, 0.0);
    vec3 dZ = vec3(0.0, hZ - h, DERIV_EPS);

    vec3 normal = normalize(cross(dZ, dX));  // orientada cap amunt

    // Espai món
    vec4 worldPos = modelMatrix * vec4(pos, 1.0);
    vWorldPos     = worldPos.xyz;

    // Normal a espai món
    vNormal = mat3(normalMatrix) * normal;

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
