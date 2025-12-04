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


//Interficies
enum class GameState { INIT, MENU, GAME, OPTIONS, LOADING };
GameState act_state = GameState::INIT; //Per veure en quin estat esta el joc
bool loading_texture_loaded = false;
GLuint textureID = 0;

void draw_image(GLuint& textureID, int weigth, int height, int x = 0, int y = 0)
{
    // Dibuixar la imatge de fons (pantalla completa)
    if (textureID != 0)
    {
        ImVec2 imagePos(x, y);
        ImVec2 imageSize(weigth, height);
        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)(intptr_t)textureID,
            imagePos,
            ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
    }
    loading_texture_loaded = true;
}

void loadTextures(const char* path)
{
    textureID = SOIL_load_OGL_texture(
        path, // Ruta relativa des de src
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS);

    if (textureID == 0)
        std::cout << "Error carregant la textura amb SOIL2: " << SOIL_last_result() << std::endl;
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
        std::cout << "Error carregant la textura amb SOIL2: " << SOIL_last_result() << std::endl;

    return texID;
}

void SetupHUD(GLFWwindow* window, int& display_h, float sizeFactorX = 0.25f, float sizeFactorY = 0.5f)
{

    int display_w;
    glfwGetWindowSize(window, &display_w, &display_h);

    ImVec2 hudSize(display_w * sizeFactorX, display_h * sizeFactorY);
    ImVec2 hudPos((display_w - hudSize.x) * 0.5f, (display_h - hudSize.y) * 0.5f);

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    draw_image(textureID, display_w, display_h);
}

void renderInici(GLFWwindow* window)
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    // Dibuixar la imatge de fons (pantalla completa)
    if (!loading_texture_loaded)
    {
        const char* path = "../images/vaixell_fons_inici_(con titulo).png";
        loadTextures(path);
        draw_image(textureID, display_w, display_h);
    }

    ImVec2 hudSize(display_w / 2.0f, display_h / 6.0f);
    ImVec2 hudPos(display_w - hudSize.x, (display_h - hudSize.y));

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Inici", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 buttonSize(display_w / 8.0f, display_h / 12.0f);

    float fontScale = display_h / 400.0f;
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0.5, 0.5));

    if (act_state == GameState::INIT)
    {
        ImGui::SameLine();
        if (ImGui::Button("Jugar", buttonSize))
            act_state = GameState::LOADING;

        ImGui::SameLine();
        if (ImGui::Button("Opcions", buttonSize))
            act_state = GameState::OPTIONS;

        ImGui::SameLine();
        if (ImGui::Button("Sortir", buttonSize)) {
            Sleep(1000);
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (ImGui::BeginPopupModal("NoDisponiblePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            ImGui::Text("Opcions encara no disponibles...");
            if (ImGui::Button("Tancar"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    ImGui::PopStyleColor();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

///////////////////////////////////////////////////////////////////////////

void renderMenu(GLFWwindow* window)
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!loading_texture_loaded)
    {
        const char* path = "../images/fons_opcions.png";
        loadTextures(path);
    }

    int display_h = 0;
    SetupHUD(window, display_h);

    ImGui::SetNextWindowBgAlpha(0.3f); // 30% opaco

    ImGui::Begin("Menu", nullptr,
        ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoTitleBar);  // treu completament la barra de títol); // <- no guarda estat

    ImVec2 actualSize = ImGui::GetWindowSize();

    ImVec2 buttonSize(actualSize.x / 2.0f, actualSize.y / 8.0f);
    float centerX = (actualSize.x - buttonSize.x) * 0.5f;

    float fontScale = display_h / 500.0f; // Escalat base (800 és referència)
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 1));

    ImGui::SetCursorPosX(centerX);
    ImGui::TextWrapped("Menu");
    ImGui::Separator();

    if (act_state == GameState::MENU) {
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Continuar", buttonSize)) act_state = GameState::GAME;

        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Pistas", buttonSize)) ImGui::OpenPopup("NoDisponiblePopup");

        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Opcions", buttonSize)) act_state = GameState::OPTIONS;

        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Sortir", buttonSize))
        {
            Sleep(1000);
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    if (ImGui::BeginPopupModal("NoDisponiblePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        ImGui::Text("Opcions encara no disponibles...");
        if (ImGui::Button("Tancar"))  ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(); //Quitar fuente y color
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//////////////////////////////

void renderOptions(GLFWwindow* window)
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!loading_texture_loaded)
    {
        const char* path = "../images/fons_opcions.png";
        loadTextures(path);
    }

    int display_h = 0;
    SetupHUD(window, display_h);

    ImGui::SetNextWindowBgAlpha(0.3f); // 30% opaco

    ImGui::Begin("Options", nullptr,
        ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoTitleBar);  // treu completament la barra de títol); // <- no guarda estat

    ImVec2 actualSize = ImGui::GetWindowSize();

    ImVec2 buttonSize(actualSize.x / 2.0f, actualSize.y / 8.0f);
    float centerX = (actualSize.x - buttonSize.x) * 0.5f;

    float fontScale = display_h / 500.0f; // Escalat base (800 és referència)
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 1));

    ImGui::SetCursorPosX(centerX);
    ImGui::TextWrapped("Menu");
    ImGui::Separator();

    ImGui::Text("Volumen de la musica ");
    ImGui::SameLine();
    if (ImGui::SliderInt("MusicVolume", &g_volume, 0, 100)) SetMusicVolume(g_volume);
    ImGui::NewLine();
    ImGui::Text("Brillantor ");
    ImGui::SameLine();

   // ImGui::SliderFloat("Brillantor", &g_DitherAmp, 0.5f, 0.7f);

    if (act_state == GameState::OPTIONS) 
    {
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Salir", buttonSize)) act_state = GameState::MENU;
    }
    ImGui::PopStyleColor(); //Quitar fuente y color
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

////////////////////////////
void renderInstruccions(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);
    ImVec2 hudSize(display_w / 2.5f, display_h / 10.0f); // Alçada: suficient per a 2 línies (per ex. 1/10 o 1/12 de l'alçada total)
    ImVec2 hudPos(0.0f, display_h - hudSize.y); // Posició: X = 0 (esquerra), Y = display_h - alçada (baix)

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);

    ImGui::SetNextWindowBgAlpha(0.2f); // 30% opaco

    ImGui::Begin("Instrucciones", nullptr,
        ImGuiWindowFlags_NoCollapse   // treu el triangle de minimitzar
        | ImGuiWindowFlags_NoTitleBar   // treu completament la barra de títol
        | ImGuiWindowFlags_NoResize     // no es pot redimensionar
        | ImGuiWindowFlags_NoMove       // no es pot moure
        | ImGuiWindowFlags_NoSavedSettings //no guarda estat
    );
    float fontScale = display_h / 700.0f; // Escalat base (800 és referència)
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1)); // Fuente y color
    ImGui::TextWrapped("WASD per mouret, SPACE saltar");
    ImGui::TextWrapped("ESC per obrir menu");
    ImGui::TextWrapped("F per tancar / obrir llanterna");

    ImGui::PopStyleColor();
    ImGui::End();
}

void renderCandelabre(GLFWwindow* window, bool& g_HeadlightEnabled)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);
    ImVec2 hudSize(display_w / 200.0f, display_h / 200.0f); // Alçada: suficient per a 2 línies (per ex. 1/10 o 1/12 de l'alçada total)
    ImVec2 hudPos(display_w, display_h - hudSize.y); // Posició: X = 0 (esquerra), Y = display_h - alçada (baix)

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hudSize, ImGuiCond_Always);

    const char* path = "../images/candelabre.png";
    if (!g_HeadlightEnabled)
        path = "../images/candelabre - tancat.png";

    loadTextures(path);
    draw_image(textureID, display_w / 6.0f, display_h / 6.0f, hudPos.x / 1.25f, hudPos.y / 1.25f);

    ImGui::SetNextWindowBgAlpha(0.0f); // 30% opaco

    ImGui::Begin("candelabre", nullptr,
        ImGuiWindowFlags_NoCollapse   // treu el triangle de minimitzar
        | ImGuiWindowFlags_NoTitleBar   // treu completament la barra de títol
        | ImGuiWindowFlags_NoResize     // no es pot redimensionar
        | ImGuiWindowFlags_NoMove       // no es pot moure
        | ImGuiWindowFlags_NoSavedSettings //no guarda estat
    );
    ImGui::End();
}

/////////////////////////////

bool renderLoading(GLFWwindow* window)
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    loading_texture_loaded = false;
    if (!loading_texture_loaded)
    {
        const char* path = "../images/vaixell_fons_inici - carregant.jpg";
        loadTextures(path);
        printf("Pantalla de cargar...");
    }

    draw_image(textureID, display_w, display_h);

    ImVec2 winSize(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(winSize);

    ImGui::Begin("Loading", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);


    ImGui::End(); // faltava tancar la finestra ImGui

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
}

//-------------------------------------------//
// Variables globals
GLuint simbol_textures[4];
bool simbols_loaded = false;
std::vector<int> ranures = { 0,0,0,0 };
std::vector<int> solucio = { 2,1,0,3 };
int numSymbols = 4;
bool cofre_on = false;
bool joc_quadres_finalitzat = false;

// Nova versió de renderPressE: mostra una imatge petita en comptes de text
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
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoSavedSettings
    );

    static GLuint pressE_texture = 0;
    if (pressE_texture == 0)
        pressE_texture = loadTextureReturnID("../images/PressE.png");


    ImVec2 imageSize(120, 64);

    ImGui::SetCursorPosX((hudSize.x - imageSize.x) * 0.5f);
    ImGui::SetCursorPosY((hudSize.y - imageSize.y) * 0.5f);

    if (pressE_texture != 0)
        ImGui::Image((void*)(intptr_t)pressE_texture, imageSize);

    ImGui::End();
}


void renderCofreContrasena(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    // Carregar les textures dels símbols només un cop
    if (!simbols_loaded)
    {
        for (int i = 0; i < numSymbols; i++)
        {
            std::string path = "../images/simbol_" + std::to_string(i) + ".png";
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



void rendePistas(GLFWwindow* window)
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!loading_texture_loaded)
    {
        const char* path = "../images/fons_opcions.png";
        loadTextures(path);
    }

    int display_h = 0;
    SetupHUD(window, display_h);

    ImGui::SetNextWindowBgAlpha(0.3f); // 30% opaco

    ImGui::Begin("Options", nullptr,
        ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoTitleBar);  // treu completament la barra de títol); // <- no guarda estat

    ImVec2 actualSize = ImGui::GetWindowSize();

    ImVec2 buttonSize(actualSize.x / 2.0f, actualSize.y / 8.0f);
    float centerX = (actualSize.x - buttonSize.x) * 0.5f;

    float fontScale = display_h / 500.0f; // Escalat base (800 és referència)
    ImGui::SetWindowFontScale(fontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 1));

    ImGui::SetCursorPosX(centerX);
    ImGui::TextWrapped("Menu");
    ImGui::Separator();

    ImGui::Text("Volumen de la musica ");
    ImGui::SameLine();
    if (ImGui::SliderInt("MusicVolume", &g_volume, 0, 100)) SetMusicVolume(g_volume);
    ImGui::NewLine();
    ImGui::Text("Brillantor ");
    ImGui::SameLine();

    // ImGui::SliderFloat("Brillantor", &g_DitherAmp, 0.5f, 0.7f);

    if (act_state == GameState::OPTIONS)
    {
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Salir", buttonSize)) act_state = GameState::MENU;
    }
    ImGui::PopStyleColor(); //Quitar fuente y color
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
