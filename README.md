# INTO THE WRATH ⚓ - Custom C++ & OpenGL 3D Engine

A 3D stylized puzzle game and custom engine built from scratch with C++ and OpenGL 4.6. 
Developed as a collaborative university project by a team of six, this engine features a self-made rendering pipeline with a signature 'Obra Dinn' 1-bit aesthetic. 
This project served as a deep dive into low-level graphics and C++ architecture, emphasizing the importance of team synchronization and effective scope management to deliver a polished product within strict time constraints.

## 🛠️ Technologies 

* Visual Studio Code
* GitHub 
* Google Drive
* Blender
* OpenGL 
* ImGui 
* GLM 
* SDL 
* GLFW

---

## 💡 My Core Contributions

As a core developer, I took ownership of critical engine subsystems and the overall game vision. My main responsibilities included:

### 1. 3D Raycasting & Interaction Engine
A precise interaction system is critical for a puzzle-focused game. I implemented a raycasting algorithm from scratch to translate 2D screen coordinates into 3D world space. 
The ray is checked against the bounding boxes of interactable entities (levers, chests, puzzle elements...) to determine the closest hit, enabling pixel-perfect selection. 
The raycaster continuously updates, modifying the UI crosshair state when hovering over valid interaction targets.

### 2. AABB Collision & Hitbox Logic
To handle player movement and environmental boundaries, I built a lightweight collision system using **Axis-Aligned Bounding Boxes (AABB)**. 
This prevents the player from clipping through the ship's geometry (walls, floors, stairs). Each interactive 3D model is assigned a customizable AABB for precise physical interactions.

### 3. Custom Audio Engine
I architected an audio subsystem directly integrated into the engine's update loop to create a highly immersive experience. 
Audio events are tied directly to the game's internal state manager. Triggers include interacting with puzzles, unlocking chests, or opening doors. 
The system separately manages constant ambient tracks (such as ocean waves) and immediate sound effects.

### 4. UI & Settings Implementation
Helped build the user interface and developer settings menus utilizing ImGui, allowing real-time adjustments of technical parameters like volume and brightness without breaking the game loop.

---

## ⚙️ Engine Architecture & Systems

The codebase is structured to separate low-level OpenGL abstraction from high-level gameplay logic.

* **Custom Render Loop:** Manages shader switching, FBO binding, and multipass rendering.
* **Entity System:** Object-oriented structure for handling 3D models, transforms, and collision triggers.
* **State Management:** A finite state machine (FSM) handles the transition between different gameplay modes (Exploration, Puzzle Interface, Minigame, Game Over).
* **Debug Tools:** Extensive integration of ImGui allows for runtime modification of shader parameters (Sobel strength, dithering matrix size, light intensity) and simplified level navigation during development.

### 2D/3D Hybrid Integration
A unique technical feature is the integration of distinct 2D elements within the 3D world, inspired by the surrealist style of *Hylics*.
* **Hand Animation System:** The first-person hands are not 3D meshes but 2D sprite sheets rendered on a quad in Normalized Device Coordinates (NDC). This ensures they always render on top of the geometry while maintaining correct aspect ratios. UV offsets are updated dynamically to play animations (clapping, holding keys, lighting candles).
* **UI Overlays:** Interfaces for maps and puzzles are rendered as overlays that pause the 3D update loop while keeping the rendering loop active.

---

## 🧩 Game Design & Level Flow

Beyond system architecture, I led the core game design and structured the logical flow of the Escape Room, which implements several distinct gameplay mechanics:

![Image](https://github.com/user-attachments/assets/c3984ec0-3463-4472-9a0e-414891c455a3)

* **Inventory & State:** An internal state manager tracks collected items (keys, tools) and validates interaction queries (like checking if the player holds a key before unlocking a door).
* **The Lever Puzzle:** An environmental puzzle where the player must correlate a 2D texture map overlay with a physical 3D array of levers, validating binary states (On/Off).
* **Symbolic Mathematics:** Puzzles requiring players to decipher numerical values from environmental textures (paintings) and input codes into combination locks.
* **"Matapatos" Minigame:** A completely separate game mode implemented within the engine featuring custom logic for target movement, collision detection by raycast, and win/lose conditions that trigger specific hand animations.
* **Inspection Minigame:** A series of multiple objects that the player must inspect in a separate view to find the codes hidden in them. 
  
### Content & Level Design
The game takes place on a multi-deck ship. The level design is vertical and interconnected:
* **Exterior:** Main deck with skybox and ocean rendering.
* **Interiors:** Corridors and rooms designed to maximize the claustrophobic effect of the dithering shader.
* **Narrative Flow:** The game features two distinct endings (Escape or "The Whale"), triggered by the player's efficiency and puzzle completion status.

---

## 🚀 Build Instructions

**Prerequisites:**
* Visual Studio 2019 or newer (Windows)
* OpenGL 4.6 compatible Graphics Card

**Steps:**
1. Clone the repository.
2. Open `IntoTheWrath.sln`.
3. Ensure the `lib` folder contains the static libraries for GLFW and GLM (included in the repo).
4. Select **x64** and **Release** configuration.
5. Build and Run.
