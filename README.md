# ChronoGL ⏰

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-5586A4?style=for-the-badge&logo=opengl&logoColor=white)

A visually appealing, interactive 3D pendulum clock, rendered in real-time using C++ and the modern OpenGL graphics pipeline. This project demonstrates core 3D graphics concepts including transformations, camera controls, and shader programming.

## 📸 Screenshot

![A beautiful 3D pendulum clock rendered in a window](screenshot.png)

## ✨ Features

- **Realistic Pendulum Physics:** Smooth and physically-based pendulum animation.
- **Interactive Orbit Camera:** Rotate, zoom, and pan around the model with your mouse.
- **Modern OpenGL:** Utilizes the core profile with custom shaders for efficient rendering.
- **Clean Code Structure:** Well-organized project with a clear separation between logic, rendering, and resources.

## 🛠️ Technologies Used

- **Language:** C++17
- **Graphics API:** OpenGL 3.3 Core
- **Libraries:**
  - [GLFW](https://www.glfw.org/) (Windowing & Input)
  - [GLAD](https://glad.dav1d.de/) (OpenGL Loading)
  - [GLM](https://glm.g-truc.net/0.9.9/index.html) (Mathematics Library for Graphics)
  - [stb_image](https://github.com/nothings/stb) (Image Loading - if you use textures)

## 🚀 How to Build and Run

This project is configured for Visual Studio 2019/2022.

### Prerequisites

1.  **Visual Studio:** Make sure you have Visual Studio with the "Desktop development with C++" workload installed.
2.  **vcpkg (Recommended):** The easiest way to install dependencies.
    ```bash
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.bat # On Windows
    ./vcpkg integrate install
    ```
3.  **Install Dependencies with vcpkg:**
    ```bash
    ./vcpkg install glfw3 glad glm
    ```

### Building

1.  Clone this repository:
    ```bash
    git clone https://github.com/Ghaithehasan/ChronoGL.git
    cd ChronoGL
    ```
2.  Open the `TestGL.sln` solution file in Visual Studio.
3.  Make sure the project configuration is set to "Release" and platform is "x64".
4.  Build the solution by pressing `Ctrl+Shift+B` or going to `Build -> Build Solution`.

### Running

1.  Ensure the `shaders/` directory is in the same location as the final executable (it should be by default).
2.  Press `F5` in Visual Studio or run the `.exe` from the `x64/Release/` directory.

## 🎮 Controls

- **W/A/S/D:** Move camera forward/left/backward/right.
- **Space / Left Ctrl:** Move camera up/down.
- **Right Mouse Button + Drag:** Rotate the camera view around the clock.
- **Mouse Scroll:** Zoom in and out.
- **ESC:** Exit the application.

## 🤝 Acknowledgments

- Inspired by the amazing [LearnOpenGL.com](https://learnopengl.com/) tutorials.
- Thanks to the authors of GLFW, GLAD, and GLM for their incredible libraries.

---
