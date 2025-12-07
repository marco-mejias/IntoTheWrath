#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/nfd.h"              
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#include "../include/gl/SOIL2.h"
#include "music.h"

// ---- Forward declarations per a gamepad + ratolí virtual ----
struct GamepadState {
    bool connected = false;

    float lx = 0.0f, ly = 0.0f;   // stick esquerre
    float rx = 0.0f, ry = 0.0f;   // stick dret
    float lt = 0.0f, rt = 0.0f;   // gatillos [0..1]

    bool btnA = false;
    bool btnB = false;
    bool btnX = false;
    bool btnY = false;
    bool btnLB = false;
    bool btnRB = false;
    bool btnBack = false;
    bool btnStart = false;
    bool btnLThumb = false;
    bool btnRThumb = false;
};


// Aquests venen de main.cpp (definits allà)
extern GamepadState g_Pad;
extern GamepadState g_PadPrev;

extern bool g_UsePadMouse;
extern bool g_InventariObert;

// Funcions definides a main.cpp
void UpdateGamepad(float dt);
void UpdatePadMouseForImGui(GLFWwindow* window);



// Interficies
enum class GameState { INIT, MENU, GAME, OPTIONS, CLUE, LOADING, END };
GameState act_state = GameState::INIT; // Per veure en quin estat està el joc

// IDs de textura per cada estat
GLuint texInit = 0;
GLuint texMenu = 0;
GLuint texOptions = 0;
GLuint texLoading = 0;


void InicialitzarImGuiFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;

    static const ImWchar ranges[] = { 0x0020, 0x00FF,0, };

    io.Fonts->AddFontFromFileTTF("../EntornVGI/Fonts/Playball-q6o1.ttf", 16.0f, &config, ranges);
}


// -------------------- Utilitats de textures --------------------

void deleteTexture(GLuint& textureID)
{
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

GLuint loadTextureReturnID(const char* path)
{
    GLuint texID = SOIL_load_OGL_texture(
        path,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS
    );

    if (texID == 0)
        std::cout << "Error carregant la textura amb SOIL2: "
        << SOIL_last_result() << std::endl;

    return texID;
}

// Carregar totes les textures al principi
void initTextures()
{
    texInit = loadTextureReturnID("../EntornVGI/textures/backgrounds/inici_vaixell.png");
    texMenu = loadTextureReturnID("../EntornVGI/textures/backgrounds/menu_llibre.png");
    texOptions = loadTextureReturnID("../EntornVGI/textures/backgrounds/menu_llibre.png");
    texLoading = loadTextureReturnID("../EntornVGI/textures/backgrounds/planol_vaixell.png");
}

// Alliberar totes les textures al final
void cleanupTextures()
{
    deleteTexture(texInit);
    deleteTexture(texMenu);
    deleteTexture(texOptions);
    deleteTexture(texLoading);
}

// -------------------- Dibuix --------------------

void draw_image(GLuint textureID, int width, int height, int x = 0, int y = 0)
{
    if (textureID != 0)
    {
        ImVec2 imagePos(x, y);
        ImVec2 imageSize(width, height);
        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)(intptr_t)textureID,
            imagePos,
            ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
    }
}

void SetupHUD(GLFWwindow* window, int& display_h,
    float sizeFactorX = 0.25f, float sizeFactorY = 0.5f)
{
    int display_w;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w * sizeFactorX, display_h * sizeFactorY);
    ImVec2 hudPos((display_w - hudSize.x) * 0.5f, (display_h - hudSize.y) * 0.5f);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
}

// -------------------- Render Inici --------------------

void renderInici(GLFWwindow* window)
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        UpdateGamepad(dtPad);
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
            UpdatePadMouseForImGui(window);
    }


    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    draw_image(texInit, display_w, display_h);

    // La finestra cobreix tota la imatge
    ImVec2 hudSize(display_w, display_h);
    ImVec2 hudPos(0, 0);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Inici", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 buttonSize(display_w / 8.0f, display_h / 14.0f);
    float fontScale = display_h / 400.0f;
    ImGui::SetWindowFontScale(fontScale);


    // Quadrant inferior dret
    float quadrantX = display_w * 0.5f;      // començament de la meitat dreta
    float quadrantY = (display_h * 0.5f) * 1.2;      // començament de la meitat inferior
    float quadrantWidth = display_w * 0.5f;
    float quadrantHeight = display_h * 0.5f;


    float spacing = 50.0f; // separació horitzontal
    float totalWidth = buttonSize.x * 3 + spacing * 2;

    // Posició inicial centrada dins del quadrant inferior dret
    float startX = quadrantX + (quadrantWidth - totalWidth) * 0.5f;
    float startY = quadrantY + (quadrantHeight - buttonSize.y) * 0.5f;

    ImGui::SetCursorPos(ImVec2(startX, startY));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 1));

    if (ImGui::Button("Jugar", buttonSize)) act_state = GameState::LOADING;

    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Sortir", buttonSize))
    {
        Sleep(1000);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
// -------------------- Render Menu --------------------

void renderMenu(GLFWwindow* window)
{
    // --- Entrada de GLFW ---
    glfwPollEvents();

    // --- Començar frame ImGui ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        // Actualitzem l'estat del gamepad (sticks + botons)
        UpdateGamepad(dtPad);

        // Al menú sempre volem poder moure el cursor amb el pad si està connectat
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
        {
            // Mou el cursor d'ImGui + el cursor del sistema amb el stick dret
            UpdatePadMouseForImGui(window);
        }
    }

    int display_w = 0, display_h = 0;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w, display_h);
    ImVec2 hudPos(0, 0);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    // Fons del menú
    draw_image(texMenu, ImGui::GetIO().DisplaySize.x, display_h);

    ImGui::Begin("Menu", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 actualSize = ImGui::GetWindowSize();
    ImVec2 buttonSize(actualSize.x / 4.0f, actualSize.y / 8.0f);

    float rightHalfX = display_w * 0.5f;
    float rightHalfWidth = display_w * 0.5f;

    float centerX = (rightHalfX + (rightHalfWidth - buttonSize.x) * 0.5f) - 20.0f;

    float offsetY = display_h * 0.12f;
    ImGui::SetCursorPosY(offsetY);

    float fontScale = display_h / 350.0f;
    ImGui::SetWindowFontScale(fontScale);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 1));

    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Continuar", buttonSize))
        act_state = GameState::GAME;
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Pistas", buttonSize))
        act_state = GameState::CLUE;
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Opcions", buttonSize))
        act_state = GameState::OPTIONS;
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Sortir", buttonSize))
    {
        Sleep(1000);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    ImGui::End();

    // --- Render final ---
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

// -------------------- Render Options --------------------

void renderOptions(GLFWwindow* window)
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        UpdateGamepad(dtPad);
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
            UpdatePadMouseForImGui(window);
    }

    int display_w = 0, display_h = 0;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w, display_h);
    ImVec2 hudPos(0, 0);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    draw_image(texMenu, ImGui::GetIO().DisplaySize.x, display_h);

    ImGui::Begin("Options", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 actualSize = ImGui::GetWindowSize();
    ImVec2 buttonSize(actualSize.x / 4.0f, actualSize.y / 8.0f);

    float rightHalfX = display_w * 0.5f;
    float rightHalfWidth = display_w * 0.5f;

    float centerX = (rightHalfX + (rightHalfWidth - buttonSize.x) * 0.5f) - 20.0f;

    float offsetY = display_h * 0.2f;
    ImGui::SetCursorPosY(offsetY);

    float fontScale = display_h / 350.0f;
    ImGui::SetWindowFontScale(fontScale);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f); // radio de redondeo en píxeles
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 1));

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::SetCursorPosX(centerX);
    ImGui::Text("Volum Musica");
    ImGui::SameLine();

    ImGui::PushItemWidth(int(actualSize.x / 6));
   // if (ImGui::SliderInt(" ", &g_volume, 0, 100))
        //SetMusicVolume(g_volume);

    ImGui::NewLine();
    ImGui::SetCursorPosX(centerX);
    ImGui::Text("Brillantor ");

    //ImGui::SliderFloat("Brillantor", &0, 0.5f, 1.0f);

    offsetY = display_h * 0.6f;
    ImGui::SetCursorPosY(offsetY);
    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Enrere", buttonSize)) act_state = GameState::MENU;

    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(9);
    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}


//----------- Extres-----
GLuint texCandelabreOn = 0;
GLuint texCandelabreOff = 0;
GLuint texPistas = 0;
GLuint texPressE = 0;
GLuint texCrono = 0;
GLuint texEnd = 0;

void initExtraTextures()
{
    texCandelabreOn = loadTextureReturnID("../EntornVGI/textures/hud/candelabre.png");
    texCandelabreOff = loadTextureReturnID("../EntornVGI/textures/hud/candelabre - tancat.png");
    texPistas = loadTextureReturnID("../EntornVGI/textures/backgrounds/llibre_vuit.png");
    texPressE = loadTextureReturnID("../images/PressE.png");
    texCrono = loadTextureReturnID("../EntornVGI/textures/hud/cronometre.png");
    texEnd = loadTextureReturnID("../EntornVGI/textures/backgrounds/end_balena.png");
}


void cleanupExtraTextures()
{
    deleteTexture(texCandelabreOn);
    deleteTexture(texCandelabreOff);
    deleteTexture(texPistas);
    deleteTexture(texPressE);
    deleteTexture(texCrono);
    deleteTexture(texEnd);
}


void renderInstruccions(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w / 2.5f, display_h / 10.0f);
    ImVec2 hudPos(0.0f, display_h - hudSize.y);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.2f);

    ImGui::Begin("Instrucciones", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);

    float fontScale = display_h / 600.0f;
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

    ImGui::TextWrapped("(ESC) per obrir menu");
    ImGui::TextWrapped("(I) obrir inventari");

    ImGui::PopStyleColor();
    ImGui::End();
}

// -------------------- Render Candelabre --------------------
void renderCandelabre(GLFWwindow* window, bool& g_HeadlightEnabled)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w / 200.0f, display_h / 200.0f);
    ImVec2 hudPos(display_w, display_h - hudSize.y);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);

    GLuint tex = g_HeadlightEnabled ? texCandelabreOn : texCandelabreOff;
    draw_image(tex, display_w / 6.0f, display_h / 6.0f,
        hudPos.x / 1.25f, hudPos.y / 1.25f);

    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("candelabre", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);
    ImGui::End();
}

// -------------------- Render Loading --------------------
bool renderLoading(GLFWwindow* window)
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        UpdateGamepad(dtPad);
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
            UpdatePadMouseForImGui(window);
    }

    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    draw_image(texLoading, display_w, display_h);

    ImVec2 winSize(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(winSize);

    ImGui::Begin("Loading", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);
    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    return true;
}

// -------------------- Render PressE --------------------
void renderPressE(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(80, 80);
    ImVec2 hudPos((display_w - hudSize.x) * 0.5f, display_h * 0.6f);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Interactuar", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);

    ImVec2 imageSize(120, 64);
    ImGui::SetCursorPosX((hudSize.x - imageSize.x) * 0.5f);
    ImGui::SetCursorPosY((hudSize.y - imageSize.y) * 0.5f);

    if (texPressE != 0)
        ImGui::Image((void*)(intptr_t)texPressE, imageSize);

    ImGui::End();
}

// ---------------Cofre contrasenya -------------

GLuint simbol_textures[4];
bool simbols_loaded = false;
std::vector<int> ranures = { 0,0,0,0 };
std::vector<int> solucio = { 2,1,0,3 };
int numSymbols = 4;
bool cofre_on = false;
bool joc_quadres_finalitzat = false;

void renderCofreContrasena(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    if (!simbols_loaded)
    {
        for (int i = 0; i < numSymbols; i++)
        {
            std::string path = "../EntornVGI/textures/minijocs/simbol_" + std::to_string(i) + ".png";
            simbol_textures[i] = loadTextureReturnID(path.c_str());
        }
        simbols_loaded = true;
    }

    // Mida i posició de la finestra HUD
    ImVec2 hudSize(display_w / 2.0f, display_h / 3.0f);
    ImVec2 hudPos(display_w / 2.0f - hudSize.x / 2.0f, display_h / 2.0f - hudSize.y / 2.0f);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Ranures", nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoSavedSettings
    );

    // Mida de cada símbol
    ImVec2 simbolSize(128, 128);

    // Centrar tot el bloc de ranures
    ImGui::SetCursorPosX((hudSize.x - (4 * simbolSize.x)) / 2.0f);

    for (int i = 0; i < 4; i++)
    {
        ImGui::BeginGroup();

        if (ImGui::ArrowButton(("##up" + std::to_string(i)).c_str(), ImGuiDir_Up))
            ranures[i] = (ranures[i] + 1) % numSymbols;

        int idx = ranures[i];
        ImGui::Image((void*)(intptr_t)simbol_textures[idx], simbolSize);

        if (ImGui::ArrowButton(("##down" + std::to_string(i)).c_str(), ImGuiDir_Down))
            ranures[i] = (ranures[i] - 1 + numSymbols) % numSymbols;

        ImGui::EndGroup();

        if (i < 3) ImGui::SameLine();
    }

    ImGui::Separator();

    // Comprovar si coincideix amb la solució
    if (ranures == solucio)
        ImGui::OpenPopup("PopupContrasenya");

    // Definir el popup modal
    if (ImGui::BeginPopupModal("PopupContrasenya", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(ImVec4(0, 0, 1, 1), "Has endevinat la contrasenya!");
        ImGui::Separator();
        PlaySoundOnce(ID_ITEM);

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            cofre_on = false; // desactivar HUD
            joc_quadres_finalitzat = true;
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

// ---------- Cronometre ---------------
float g_DuradaCronometre = 15.0f * 60.0f;
float g_TempsTranscorregut = 0.0f;

void renderCronometre(GLFWwindow* window)
{
    float tempsRestant = glm::max(0.0f, g_DuradaCronometre - g_TempsTranscorregut);

    int display_w = 0, display_h = 0;
    glfwGetWindowSize(window, &display_w, &display_h);

    int minuts = static_cast<int>(tempsRestant) / 60;
    int segons = static_cast<int>(tempsRestant) % 60;

    // La finestra ocupa tota la pantalla
    ImVec2 cronoPos(0, 0);
    ImVec2 cronoSize(display_w, display_h);

    ImGui::SetNextWindowPos(cronoPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(cronoSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (ImGui::Begin("Cronometre", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings))
    {
        ImVec2 imageSize(display_w / 7.0f, display_h / 4.2f); // mida de la imatge
        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::Image((void*)(intptr_t)texCrono, imageSize);

        ImVec2 textSize = ImGui::CalcTextSize("00:00");
        float textX = (imageSize.x - textSize.x) * 0.5f;
        float textY = (imageSize.y - textSize.y) * 0.65f;
        ImGui::SetCursorPos(ImVec2(textX, textY));

        float fontScale = display_h / 500.0f;
        ImGui::SetWindowFontScale(fontScale);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        ImGui::Text("%02d:%02d", minuts, segons);
        ImGui::PopStyleColor();
    }
    ImGui::End();

    if (g_TempsTranscorregut >= g_DuradaCronometre)
        act_state = GameState::END;
}

void renderFinalJoc(GLFWwindow* window)
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        UpdateGamepad(dtPad);
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
            UpdatePadMouseForImGui(window);
    }


    int display_h = 0;
    SetupHUD(window, display_h);

    draw_image(texEnd, ImGui::GetIO().DisplaySize.x, display_h);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f); // transparent

    ImGui::Begin("End", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 actualSize = ImGui::GetWindowSize();
    ImVec2 buttonSize(actualSize.x / 4.0f, actualSize.y / 10.0f);

    // Escalat de font més gran
    float fontScale = display_h / 250.0f;
    ImGui::SetWindowFontScale(fontScale);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 1));

    // --- Centrat vertical i horitzontal ---
    ImVec2 textSize = ImGui::CalcTextSize("Has Perdut!");
    float totalHeight = textSize.y + 20.0f + buttonSize.y; // text + espai + botó
    float startY = (actualSize.y - totalHeight) * 0.5f;

    // Text centrat
    ImGui::SetCursorPos(ImVec2((actualSize.x - textSize.x) * 0.5f, startY));
    ImGui::Text("Has Perdut!");

    // Botó centrat sota el text
    ImGui::SetCursorPos(ImVec2((actualSize.x - buttonSize.x) * 0.5f, startY + textSize.y + 20.0f));
    if (ImGui::Button("Sortir", buttonSize))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}




int idx_clue = 1;
// -------------------- Render Pistas --------------------
void renderPistas(GLFWwindow* window)
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        float dtPad = io.DeltaTime;
        if (dtPad <= 0.0f) dtPad = 1.0f / 60.0f;

        UpdateGamepad(dtPad);
        g_UsePadMouse = g_Pad.connected;

        if (g_UsePadMouse)
            UpdatePadMouseForImGui(window);
    }
    int display_w = 0, display_h = 0;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w, display_h);
    ImVec2 hudPos(0, 0);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    draw_image(texPistas, ImGui::GetIO().DisplaySize.x, display_h);

    ImGui::Begin("Pistes", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 actualSize = ImGui::GetWindowSize();
    ImVec2 buttonSize(actualSize.x / 4.0f, actualSize.y / 8.0f);

    float rightHalfX = display_w * 0.5f;
    float rightHalfWidth = display_w * 0.5f;

    float centerX = (rightHalfX + (rightHalfWidth - buttonSize.x) * 0.5f) - 120.0f;

    float offsetY = display_h * 0.2f;
    ImGui::SetCursorPosY(offsetY);

    float fontScale = display_h / 350.0f;
    ImGui::SetWindowFontScale(fontScale);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f); // radio de redondeo en píxeles
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 1));


    ImGui::SetCursorPosX(centerX);
    ImGui::Text("Pista %d", idx_clue);
    ImGui::Spacing();

    if (idx_clue == 1)
    {
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("La habitacio esta tancada amb clau.");
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("Hi ha un cofre amb contrasenya,");
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("potser si trobo la contrasenya amb");
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("podre sortir d'aqui.");

    }
    else if (idx_clue == 2)
    {
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("Una clau rovellada.");
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("Sembla que coincideix amb una de les");
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("portes.");
    }

    offsetY = display_h * 0.6f;
    ImGui::SetCursorPosY(offsetY);
    ImGui::SetCursorPosX(centerX);
    if (ImGui::Button("Enrere", buttonSize)) act_state = GameState::MENU;

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
