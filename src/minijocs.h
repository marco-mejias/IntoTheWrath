//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <vector>
//#include <algorithm>
//#include <random>
//#include <cstdio>
//
//// ----------------------------- Configuració mínima i globals del minijoc -----------------------------
//enum class EstatPalanques { OFF, PREPARANT, JUGANT, ACABAT };
//
//struct Objectiu {
//    glm::vec3 posicioBase;
//    glm::vec3 posicioActual;
//    glm::vec3 mida;
//    float fase;
//    float velocitat;
//    bool viu;
//};
//
//static EstatPalanques g_EstatPalanques = EstatPalanques::OFF;
//static std::vector<Objectiu> g_ObjectiusPalanques;
//static std::vector<int> g_PalanquesOrder;   // ordre a prémer (indices 0..3)
//static int g_PalanquesOrderIndex = 0;
//static bool g_PalanquesButtonsActive = false;
//static float g_PalanquesButtonRadius = 0.2f;
//static glm::vec3 g_PalanquesParetCentre(0.0f);
//static float g_PalanquesAmplada = 2.0f;
//static float g_PalanquesAlcada = 1.0f;
//static int g_PuntsPalanques = 0;
//static int g_PuntsObjectiuPalanques = 4;
//static bool g_PalanquesSuperat = false;
//static bool g_PalanquesRewardDonat = false;
//
//// Geometria (cub)
//static GLuint g_PalanquesVAO = 0;
//static GLuint g_PalanquesVBO = 0;
//
//// Variables d'entrada
//static bool g_KeyUPrev = false;
//static bool g_MouseLeftPrev = false;
//
//// Camera / jugador (si al teu projecte ja existeixen, ajusta o reemplaça amb externs)
//static glm::vec3 g_FPVPos = glm::vec3(0.0f, 1.6f, 0.0f);
//static float g_FPVYaw = 90.0f;    // graus
//static float g_FPVPitch = 0.0f;   // graus
//
//// Matrius de view/projection (cal actualitzar-les des del teu pipeline abans de RenderPalanques)
//static glm::mat4 g_ViewMatrix = glm::mat4(1.0f);
//static glm::mat4 g_ProjectionMatrix = glm::mat4(1.0f);
//
//// Shader mínim per dibuixar cubs (simple color + matrices)
//// Aquest shader és molt bàsic; si tens el teu shader, fes servir-lo i omet la part de compilació del shader.
//static const char* vs_src =
//"#version 330 core\n"
//"layout(location=0) in vec3 aPos;\n"
//"layout(location=1) in vec3 aNormal;\n"
//"uniform mat4 modelMatrix;\n"
//"uniform mat4 viewMatrix;\n"
//"uniform mat4 projectionMatrix;\n"
//"void main() { gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos,1.0); }\n";
//
//static const char* fs_src =
//"#version 330 core\n"
//"out vec4 FragColor;\n"
//"uniform vec3 uColor;\n"
//"void main(){ FragColor = vec4(uColor,1.0); }\n";
//
//static GLuint g_PalanquesProg = 0;
//
//// ----------------------------- Utilitats internes -----------------------------
//static GLuint CompileShader(GLenum type, const char* src)
//{
//    GLuint s = glCreateShader(type);
//    glShaderSource(s, 1, &src, nullptr);
//    glCompileShader(s);
//    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
//    if (!ok) { char buf[1024]; glGetShaderInfoLog(s, 1024, nullptr, buf); fprintf(stderr, "Shader compile error: %s\n", buf); }
//    return s;
//}
//
//static GLuint CreateProgramSimple()
//{
//    GLuint vs = CompileShader(GL_VERTEX_SHADER, vs_src);
//    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fs_src);
//    GLuint p = glCreateProgram();
//    glAttachShader(p, vs); glAttachShader(p, fs);
//    glLinkProgram(p);
//    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
//    if (!ok) { char buf[1024]; glGetProgramInfoLog(p, 1024, nullptr, buf); fprintf(stderr, "Program link error: %s\n", buf); }
//    glDeleteShader(vs); glDeleteShader(fs);
//    return p;
//}
//
//// Ray vs AABB intersection (slab method)
//// origen dir han de ser en el mateix espai que els bounds
//
//// Calcula direcció del ray a partir de yaw/pitch (igual que en FPV)
//static glm::vec3 CameraDirFromYawPitch(float yawDeg, float pitchDeg)
//{
//    float cy = cosf(glm::radians(yawDeg));
//    float sy = sinf(glm::radians(yawDeg));
//    float cp = cosf(glm::radians(pitchDeg));
//    float sp = sinf(glm::radians(pitchDeg));
//    return glm::normalize(glm::vec3(cy * cp, sp, sy * cp));
//}
//
//// ----------------------------- Geometria del cub (VAO/VBO) -----------------------------
//void InitPalanquesGeometry()
//{
//    if (g_PalanquesVAO != 0) return;
//
//    struct Vert { float px, py, pz; float nx, ny, nz; };
//    Vert verts[] = {
//        // +X
//        { +0.5f, -0.5f, -0.5f,  +1,0,0 }, { +0.5f, +0.5f, -0.5f,  +1,0,0 }, { +0.5f, +0.5f, +0.5f,  +1,0,0 },
//        { +0.5f, -0.5f, -0.5f,  +1,0,0 }, { +0.5f, +0.5f, +0.5f,  +1,0,0 }, { +0.5f, -0.5f, +0.5f,  +1,0,0 },
//        // -X
//        { -0.5f, -0.5f, +0.5f,  -1,0,0 }, { -0.5f, +0.5f, +0.5f,  -1,0,0 }, { -0.5f, +0.5f, -0.5f,  -1,0,0 },
//        { -0.5f, -0.5f, +0.5f,  -1,0,0 }, { -0.5f, +0.5f, -0.5f,  -1,0,0 }, { -0.5f, -0.5f, -0.5f,  -1,0,0 },
//        // +Y
//        { -0.5f, +0.5f, -0.5f,  0,1,0 }, { -0.5f, +0.5f, +0.5f,  0,1,0 }, { +0.5f, +0.5f, +0.5f,  0,1,0 },
//        { -0.5f, +0.5f, -0.5f,  0,1,0 }, { +0.5f, +0.5f, +0.5f,  0,1,0 }, { +0.5f, +0.5f, -0.5f,  0,1,0 },
//        // -Y
//        { -0.5f, -0.5f, +0.5f,  0,-1,0 }, { -0.5f, -0.5f, -0.5f,  0,-1,0 }, { +0.5f, -0.5f, -0.5f,  0,-1,0 },
//        { -0.5f, -0.5f, +0.5f,  0,-1,0 }, { +0.5f, -0.5f, -0.5f,  0,-1,0 }, { +0.5f, -0.5f, +0.5f,  0,-1,0 },
//        // +Z
//        { -0.5f, -0.5f, +0.5f,  0,0,1 }, { +0.5f, -0.5f, +0.5f,  0,0,1 }, { +0.5f, +0.5f, +0.5f,  0,0,1 },
//        { -0.5f, -0.5f, +0.5f,  0,0,1 }, { +0.5f, +0.5f, +0.5f,  0,0,1 }, { -0.5f, +0.5f, +0.5f,  0,0,1 },
//        // -Z
//        { +0.5f, -0.5f, -0.5f,  0,0,-1 }, { -0.5f, -0.5f, -0.5f,  0,0,-1 }, { -0.5f, +0.5f, -0.5f,  0,0,-1 },
//        { +0.5f, -0.5f, -0.5f,  0,0,-1 }, { -0.5f, +0.5f, -0.5f,  0,0,-1 }, { +0.5f, +0.5f, -0.5f,  0,0,-1 },
//    };
//
//    glGenVertexArrays(1, &g_PalanquesVAO);
//    glGenBuffers(1, &g_PalanquesVBO);
//
//    glBindVertexArray(g_PalanquesVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, g_PalanquesVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
//
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)0);
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(3 * sizeof(float)));
//
//    glBindVertexArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
//
//// ----------------------------- Lògica del minijoc -----------------------------
//static float RandFloat(float a, float b)
//{
//    static std::mt19937 mt{ std::random_device{}() };
//    std::uniform_real_distribution<float> dist(a, b);
//    return dist(mt);
//}
//
//void InitPalanques(const glm::vec3& panelCenter, float panelWidth, float panelHeight)
//{
//    InitPalanquesGeometry();
//
//    g_PalanquesParetCentre = panelCenter;
//    g_PalanquesAmplada = panelWidth;
//    g_PalanquesAlcada = panelHeight;
//
//    g_ObjectiusPalanques.clear();
//    g_ObjectiusPalanques.resize(4);
//
//    float x0 = -panelWidth * 0.45f;
//    float x1 = -panelWidth * 0.15f;
//    float x2 = panelWidth * 0.15f;
//    float x3 = panelWidth * 0.45f;
//    float y = 0.0f;
//
//    glm::vec3 offsets[4] = { {x0,y,0},{x1,y,0},{x2,y,0},{x3,y,0} };
//    for (int i = 0; i < 4; ++i) {
//        g_ObjectiusPalanques[i].posicioBase = panelCenter + offsets[i] + glm::vec3(0.0f, 0.8f, 0.0f); // elevat
//        g_ObjectiusPalanques[i].posicioActual = g_ObjectiusPalanques[i].posicioBase;
//        g_ObjectiusPalanques[i].mida = glm::vec3(panelWidth * 0.18f, panelHeight * 0.6f, 0.12f);
//        g_ObjectiusPalanques[i].fase = RandFloat(0.0f, 6.28f);
//        g_ObjectiusPalanques[i].velocitat = 1.0f + 0.2f * i;
//        g_ObjectiusPalanques[i].viu = true;
//    }
//
//    g_PalanquesOrder = { 0,1,2,3 };
//    std::shuffle(g_PalanquesOrder.begin(), g_PalanquesOrder.end(), std::mt19937{ std::random_device{}() });
//    g_PalanquesOrderIndex = 0;
//    g_PalanquesButtonsActive = false;
//    g_PuntsPalanques = 0;
//    g_PalanquesSuperat = false;
//    g_PalanquesRewardDonat = false;
//    g_EstatPalanques = EstatPalanques::PREPARANT;
//}
//
//void StartPalanques()
//{
//    if (g_EstatPalanques == EstatPalanques::JUGANT) return;
//    g_EstatPalanques = EstatPalanques::JUGANT;
//    g_PalanquesOrderIndex = 0;
//    g_PalanquesButtonsActive = true;
//    for (auto& o : g_ObjectiusPalanques) o.viu = true;
//}
//
//// Actualitzar animacions i temporitzador (si n'hi ha)
//void UpdatePalanques(float dt)
//{
//    if (g_EstatPalanques != EstatPalanques::JUGANT) return;
//    for (auto& o : g_ObjectiusPalanques) {
//        o.fase += o.velocitat * dt;
//        float amp = 0.03f;
//        o.posicioActual = o.posicioBase + glm::vec3(0.0f, amp * sinf(o.fase), 0.0f);
//    }
//}
//
//// Render: pinta els 4 blocs amb un color segons estat
//void RenderPalanques()
//{
//    if (g_EstatPalanques == EstatPalanques::OFF) return;
//    if (!g_PalanquesVAO) return;
//    if (!g_PalanquesProg) g_PalanquesProg = CreateProgramSimple();
//
//    glUseProgram(g_PalanquesProg);
//    GLint locModel = glGetUniformLocation(g_PalanquesProg, "modelMatrix");
//    GLint locView = glGetUniformLocation(g_PalanquesProg, "viewMatrix");
//    GLint locProj = glGetUniformLocation(g_PalanquesProg, "projectionMatrix");
//    GLint locColor = glGetUniformLocation(g_PalanquesProg, "uColor");
//
//    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(g_ViewMatrix));
//    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(g_ProjectionMatrix));
//
//    glBindVertexArray(g_PalanquesVAO);
//    for (int i = 0; i < (int)g_ObjectiusPalanques.size(); ++i) {
//        const Objectiu& o = g_ObjectiusPalanques[i];
//        if (!o.viu) continue;
//
//        glm::vec3 color(0.8f, 0.6f, 0.2f);
//        if (g_PalanquesButtonsActive && g_PalanquesOrderIndex < (int)g_PalanquesOrder.size() && g_PalanquesOrder[g_PalanquesOrderIndex] == i)
//            color = glm::vec3(0.2f, 1.0f, 0.2f); // següent esperat
//        glUniform3fv(locColor, 1, glm::value_ptr(color));
//
//        glm::mat4 M(1.0f);
//        M = glm::translate(M, o.posicioActual);
//        M = glm::scale(M, o.mida);
//        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(M));
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//    }
//    glBindVertexArray(0);
//    // No glUseProgram(0) per eficàcia; el caller pot canviar-lo
//}
//
//// Comprovació AABB per a hit test (es fa amb ray del jugador)
//static bool IsHitOnButton(int idx, const glm::vec3& hitWorld)
//{
//    if (idx < 0 || idx >= (int)g_ObjectiusPalanques.size()) return false;
//    const Objectiu& o = g_ObjectiusPalanques[idx];
//    glm::vec3 half = o.mida * 0.5f;
//    glm::vec3 d = hitWorld - o.posicioActual;
//    return (fabs(d.x) <= half.x && fabs(d.y) <= half.y && fabs(d.z) <= half.z + 0.05f);
//}
//
//// Handle mouse click; cal cridar-ho des del teu callback OnMouseButton o polleig manual.
//// Aquí fem ray des del jugador segons yaw/pitch i mirem intersecció amb cada AABB.
//void HandleMouseClickPalanques(GLFWwindow* window, int mouseButton, int action)
//{
//    if (g_EstatPalanques != EstatPalanques::JUGANT) { g_MouseLeftPrev = false; return; }
//    if (!g_PalanquesButtonsActive) return;
//    if (mouseButton != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
//
//    glm::vec3 orig = g_FPVPos;
//    glm::vec3 dir = CameraDirFromYawPitch(g_FPVYaw, g_FPVPitch);
//
//    for (int i = 0; i < (int)g_ObjectiusPalanques.size(); ++i) {
//        if (!g_ObjectiusPalanques[i].viu) continue;
//        glm::vec3 half = g_ObjectiusPalanques[i].mida * 0.5f;
//        glm::vec3 bmin = g_ObjectiusPalanques[i].posicioActual - half;
//        glm::vec3 bmax = g_ObjectiusPalanques[i].posicioActual + half;
//        float t;
//        if (RayIntersectsAABB(orig, dir, bmin, bmax, &t)) {
//            // Hem colpejat el bloc i -> comprovar ordre
//            int expected = g_PalanquesOrder[g_PalanquesOrderIndex];
//            if (i == expected) {
//                g_PuntsPalanques += 1;
//                g_ObjectiusPalanques[i].viu = false;
//                g_PalanquesOrderIndex++;
//                if (g_PalanquesOrderIndex >= (int)g_PalanquesOrder.size()) {
//                    g_PalanquesSuperat = true;
//                    g_EstatPalanques = EstatPalanques::ACABAT;
//                    g_PalanquesButtonsActive = false;
//                }
//            }
//            else {
//                // Penalització opcional: reiniciar ordre i reactivar tots
//                g_PuntsPalanques = std::max(0, g_PuntsPalanques - 1);
//                g_PalanquesOrderIndex = 0;
//                for (auto& o : g_ObjectiusPalanques) o.viu = true;
//            }
//            break; // només un objectiu per clic
//        }
//    }
//}
//
//// Spawnar 4 blocs a la dreta del jugador quan premem U
//void SpawnPalanquesNearPlayer()
//{
//    glm::vec3 panelCenter = g_FPVPos + glm::vec3(1.0f, 0.0f, 0.0f);
//    InitPalanques(panelCenter, g_PalanquesAmplada, g_PalanquesAlcada);
//    StartPalanques();
//}
//
//// Entrades: cridar cada frame per pollejar U (o integra-ho al teu sistema d'input)
//void HandleKeySpawnPalanques(GLFWwindow* window)
//{
//    bool keyDown = (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS);
//    if (keyDown && !g_KeyUPrev) SpawnPalanquesNearPlayer();
//    g_KeyUPrev = keyDown;
//}
//
//// ----------------------------- Exemple d'integració mínima -----------------------------
//// Afegeix aquestes crides al teu main loop d'entrada/actualització/render:
////
//// // inicialitzar matrices de camera/viewport amb les teves valors reals:
//// g_ViewMatrix = ViewMatrix;              // aquí la teva ViewMatrix
//// g_ProjectionMatrix = ProjectionMatrix;  // la teva ProjectionMatrix
////
//// // polleig d'entrada cada frame:
//// HandleKeySpawnPalanques(window);
//// // si vols support per clicks mitjançant callback GLFW, crida HandleMouseClickPalanques des del teu OnMouseButton:
//// //   ex: if (button==GLFW_MOUSE_BUTTON_LEFT) HandleMouseClickPalanques(window, button, action);
////
//// // update:
//// UpdatePalanques(deltaTime);
////
//// // render (després de configurar shader/view/proj globals):
//// RenderPalanques();
////
//// ----------------------------- Neteja -----------------------------
//void ShutdownPalanques()
//{
//    if (g_PalanquesVBO) { glDeleteBuffers(1, &g_PalanquesVBO); g_PalanquesVBO = 0; }
//    if (g_PalanquesVAO) { glDeleteVertexArrays(1, &g_PalanquesVAO); g_PalanquesVAO = 0; }
//    if (g_PalanquesProg) { glDeleteProgram(g_PalanquesProg); g_PalanquesProg = 0; }
//}
//
//
//
////----------------------//
////---Mata patos---------//
////----------------------//
