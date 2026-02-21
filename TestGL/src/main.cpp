// =============================================================
// SolarSystem3D - main.cpp
// Entry point for the 3D Solar System simulation using OpenGL.
// Handles initialization, the main render loop, orbital mechanics,
// eclipse detection, and user input.
// =============================================================

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "Sphere.h"
#include "TextureLoader.h"
#include "Skybox.h"
#include "OrbitPath.h"

// Window resolution
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera starting position: slightly above and far back to see the full system
Camera camera(glm::vec3(0.0f, 30.0f, 80.0f));
float lastX = SCR_WIDTH / 2.0f;  // Last known mouse X position
float lastY = SCR_HEIGHT / 2.0f; // Last known mouse Y position
bool firstMouse = true;          // Flag to ignore first mouse jump

// Delta time for frame-rate-independent movement
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Current orbital and rotation angles for each body (in radians)
float earthOrbitAngle = 0.0f;
float moonOrbitAngle = 0.0f;
float marsOrbitAngle = 0.0f;
float earthRotationAngle = 0.0f; // Earth's self-rotation around Y axis

// Simulation speed controls
float timeSpeed = 0.5f;       // Current speed multiplier
float normalTimeSpeed = 0.5f; // Default (normal) speed
float fastTimeSpeed = 4.0f;   // Fast-forward speed for eclipse search

// Eclipse state flags
bool isEclipse = false;           // True when a solar eclipse is active
bool isLunarEclipse = false;      // True when a lunar eclipse is active
bool speedUpMode = false;         // Fast-forward mode searching for solar eclipse
bool speedUpModeLunar = false;    // Fast-forward mode searching for lunar eclipse

// Camera follow mode
bool cameraFollowEarth = false;   // If true, camera tracks Earth's position

// Adjusted moon position used during eclipse snap
glm::vec3 adjustedMoonPos;
bool moonPosAdjusted = false;     // True when moon has been snapped to eclipse position

// Radii of celestial bodies (scene units)
const float SUN_RADIUS = 10.0f;
const float EARTH_RADIUS = 3.0f;
const float MOON_RADIUS = 2.4f;
const float MARS_RADIUS = 1.5f;

// Orbital parameters: elliptical semi-major and semi-minor axes
const float EARTH_ORBIT_SEMI_MAJOR = 60.0f;
const float EARTH_ORBIT_SEMI_MINOR = 55.0f;
const float MOON_ORBIT_RADIUS = 12.0f;      // Moon orbits Earth at this radius
const float MARS_ORBIT_SEMI_MAJOR = 85.0f;
const float MARS_ORBIT_SEMI_MINOR = 80.0f;

// Base colors for each celestial body (used as fallback if no texture)
glm::vec3 sunColor(1.0f, 0.95f, 0.8f);     // Warm yellow-white
glm::vec3 earthColor(0.15f, 0.5f, 0.7f);   // Blue-green
glm::vec3 moonColor(0.75f, 0.75f, 0.8f);   // Grey-blue
glm::vec3 marsColor(0.8f, 0.3f, 0.2f);     // Red-orange

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

glm::vec3 calculateEarthPosition(float angle);
glm::vec3 calculateMoonPosition(glm::vec3 earthPos, float angle);
glm::vec3 calculateMarsPosition(float angle);
bool checkSolarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos);
bool checkLunarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos);

int main() {
    // Initialize GLFW and request OpenGL 3.3 Core Profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the application window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System - Earth, Moon & Sun", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Register window/input callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Hide and capture the cursor for FPS-style camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load all OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing so closer objects occlude farther ones
    glEnable(GL_DEPTH_TEST);
    // Cull back faces to improve rendering performance
    glEnable(GL_CULL_FACE);

    // Load GLSL shaders from disk
    Shader solarShader("shaders/solar_vertex.glsl", "shaders/solar_fragment.glsl");   // Used for planets
    Shader skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl"); // Used for starfield background
    Shader orbitShader("shaders/orbit_vertex.glsl", "shaders/orbit_fragment.glsl");   // Used for orbit path lines

    // Load planet textures (false = no gamma correction)
    unsigned int sunTexture = TextureLoader::loadTexture("textures/8k_sun.jpg", false);
    unsigned int earthDayTexture = TextureLoader::loadTexture("textures/2k_earth_daymap.jpg", false);
    unsigned int earthNightTexture = TextureLoader::loadTexture("textures/2k_earth_nightmap.jpg", false);
    unsigned int earthCloudsTexture = TextureLoader::loadTexture("textures/2k_earth_clouds.jpg", false);
    unsigned int moonTexture = TextureLoader::loadTexture("textures/2k_moon.jpg", false);
    unsigned int marsTexture = TextureLoader::loadTexture("textures/8k_mars.jpg", false);

    // Create and load the starfield skybox
    Skybox skybox;
    skybox.loadTexture("textures/2k_stars_milky_way.jpg");

    // Generate elliptical orbit path for Earth (120 segments for smoothness)
    OrbitPath earthOrbitPath;
    earthOrbitPath.generateEarthOrbit(EARTH_ORBIT_SEMI_MAJOR, EARTH_ORBIT_SEMI_MINOR, 120);
    
    // Generate circular orbit path for the Moon around Earth (80 segments)
    OrbitPath moonOrbitPath;
    moonOrbitPath.generateMoonOrbit(MOON_ORBIT_RADIUS, 80);

    // Create sphere meshes for each celestial body (radius, latitude segments, longitude segments)
    Sphere sun(SUN_RADIUS, 50, 50);
    Sphere earth(EARTH_RADIUS, 40, 40);
    Sphere moon(MOON_RADIUS, 30, 30);
    Sphere mars(MARS_RADIUS, 35, 35);

    // ===================== Main Render Loop =====================
    while (!glfwWindowShouldClose(window)) {
        // Calculate time elapsed since last frame
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle keyboard/mouse input
        processInput(window);

        // Update orbital angles only when no eclipse is active
        if (!isEclipse && !isLunarEclipse) {
            // Clamp deltaTime to avoid huge jumps on slow frames
            float clampedDeltaTime = std::min(deltaTime, 0.1f);
            earthOrbitAngle    += 0.3f  * timeSpeed * clampedDeltaTime; // Earth orbits slowest
            moonOrbitAngle     += 1.2f  * timeSpeed * clampedDeltaTime; // Moon orbits fastest
            marsOrbitAngle     += 0.15f * timeSpeed * clampedDeltaTime; // Mars orbits slowest
            earthRotationAngle += 2.0f  * timeSpeed * clampedDeltaTime; // Earth spins on its axis
        }

        // Compute world-space positions for each body
        glm::vec3 sunPos(0.0f, 0.0f, 0.0f); // Sun is fixed at origin
        glm::vec3 earthPos = calculateEarthPosition(earthOrbitAngle);
        glm::vec3 moonPos;
        // Use snapped moon position during eclipse, otherwise calculate normally
        if (moonPosAdjusted && (isEclipse || isLunarEclipse)) {
            moonPos = adjustedMoonPos;
        } else {
            moonPos = calculateMoonPosition(earthPos, moonOrbitAngle);
            moonPosAdjusted = false;
        }
        glm::vec3 marsPos = calculateMarsPosition(marsOrbitAngle);

        // Camera follow mode: place camera near Earth looking at the Sun or Moon
        if (cameraFollowEarth) {
            glm::vec3 lookTarget;
            // During solar eclipse, look toward the Moon (blocking the Sun)
            if (isEclipse) {
                lookTarget = moonPos;
            } else {
                lookTarget = sunPos;
            }
            
            glm::vec3 direction = glm::normalize(lookTarget - earthPos);
            // Position camera just outside Earth's surface in the direction of the target
            glm::vec3 cameraPos = earthPos - direction * EARTH_RADIUS;
            
            camera.SetPositionAndLookAt(cameraPos, lookTarget);
        }

        // Solar eclipse detection: check alignment when speed-up mode is active
        if (speedUpMode && !isEclipse && !isLunarEclipse) {
            isEclipse = checkSolarEclipse(sunPos, earthPos, moonPos);
            if (isEclipse) {
                // Snap the Moon to perfect alignment between Sun and Earth
                glm::vec3 sunToEarth = earthPos - sunPos;
                float sunToEarthDist = glm::length(sunToEarth);
                glm::vec3 sunToEarthDir = glm::normalize(sunToEarth);
                
                float moonDistFromEarth = MOON_ORBIT_RADIUS;
                float moonDistFromSun = sunToEarthDist - moonDistFromEarth;
                
                if (moonDistFromSun > 0 && moonDistFromSun < sunToEarthDist) {
                    adjustedMoonPos = sunPos + sunToEarthDir * moonDistFromSun;
                    adjustedMoonPos.y = 0.0f; // Keep everything on the orbital plane
                    moonPosAdjusted = true;
                    
                    // Verify alignment using dot product (should be ~1.0 for perfect alignment)
                    glm::vec3 verifySunToMoon = adjustedMoonPos - sunPos;
                    glm::vec3 verifyMoonToEarth = earthPos - adjustedMoonPos;
                    float verifyAlignment = glm::dot(glm::normalize(verifySunToMoon), glm::normalize(verifyMoonToEarth));
                    
                    std::cout << "SOLAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                    std::cout << "Alignment verification: " << verifyAlignment << " (should be ~1.0)" << std::endl;
                }
                
                // Stop simulation time to hold the eclipse position
                timeSpeed = 0.0f;
            }
        }

        // Lunar eclipse detection: check alignment when lunar speed-up mode is active
        if (speedUpModeLunar && !isLunarEclipse && !isEclipse) {
            isLunarEclipse = checkLunarEclipse(sunPos, earthPos, moonPos);
            if (isLunarEclipse) {
                // Snap Moon directly behind Earth (opposite side from Sun)
                glm::vec3 sunToEarth = glm::normalize(earthPos - sunPos);
                float earthToMoonDist = MOON_ORBIT_RADIUS;
                moonPos = earthPos + sunToEarth * earthToMoonDist;
                moonPos.y = 0.0f;
                
                std::cout << "LUNAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                timeSpeed = 0.0f;
            }
        }

        // Clear screen with black background (space)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- Draw Skybox ----
        // Use GL_LEQUAL so the skybox passes depth test at maximum depth
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        // Remove translation from view matrix so skybox stays stationary
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        skyboxShader.setInt("skybox", 0);
        skybox.Draw();
        glDepthFunc(GL_LESS); // Restore default depth function

        // ---- Draw Orbit Paths ----
        // Enable alpha blending so orbit lines can be semi-transparent
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.5f);
        orbitShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        orbitShader.setMat4("projection", projection);
        orbitShader.setMat4("view", view);
        
        // Earth's orbit path (centered at origin)
        glm::mat4 model = glm::mat4(1.0f);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.8f, 0.8f, 0.9f));
        earthOrbitPath.Draw();
        
        // Moon's orbit path (translated to follow Earth's current position)
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.7f, 0.7f, 0.8f));
        moonOrbitPath.Draw();
        
        glDisable(GL_BLEND);
        glLineWidth(1.0f);

        // ---- Draw Celestial Bodies ----
        solarShader.use();

        // Set shared matrices and camera position for lighting calculations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        solarShader.setMat4("projection", projection);
        solarShader.setMat4("view", view);
        solarShader.setVec3("viewPos", camera.Position);

        // -- Draw Sun --
        model = glm::mat4(1.0f);
        model = glm::translate(model, sunPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", sunColor);
        solarShader.setInt("objectType", 0); // 0 = Sun (emissive, no lighting)
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", false);
        solarShader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        solarShader.setInt("diffuseTexture", 0);
        sun.Draw();
            
        // -- Draw Earth --
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
        // Apply axial self-rotation around the Y axis
        model = glm::rotate(model, earthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", earthColor);
        solarShader.setInt("objectType", 1); // 1 = Planet (receives lighting)
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", false);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", true);   // Blend night-side city lights
        solarShader.setBool("useCloudsTexture", true);  // Overlay cloud layer
        
        // Bind day, night, and cloud textures to separate texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthDayTexture);
        solarShader.setInt("diffuseTexture", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earthNightTexture);
        solarShader.setInt("nightTexture", 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earthCloudsTexture);
        solarShader.setInt("cloudsTexture", 2);
        
        earth.Draw();

        // -- Draw Moon --
        model = glm::mat4(1.0f);
        model = glm::translate(model, moonPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", moonColor);
        solarShader.setInt("objectType", 2); // 2 = Moon (special lighting)
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", true);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", false);  // Moon has no night texture
        solarShader.setBool("useCloudsTexture", false); // Moon has no clouds
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        solarShader.setInt("diffuseTexture", 0);
        moon.Draw();

        // -- Draw Mars --
        model = glm::mat4(1.0f);
        model = glm::translate(model, marsPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", marsColor);
        solarShader.setInt("objectType", 1); // 1 = Planet
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", false);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", false);  // Mars has no night lights
        solarShader.setBool("useCloudsTexture", false); // Mars has no cloud layer
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, marsTexture);
        solarShader.setInt("diffuseTexture", 0);
        mars.Draw();

        // Swap front and back buffers to display the rendered frame
        glfwSwapBuffers(window);
        // Process pending window/input events
        glfwPollEvents();
    }

    // Clean up GLFW resources before exit
    glfwTerminate();
    return 0;
}

// Returns the world position of Earth on its elliptical orbit given an angle
glm::vec3 calculateEarthPosition(float angle) {
    float x = EARTH_ORBIT_SEMI_MAJOR * cos(angle);
    float z = EARTH_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

// Returns the world position of the Moon relative to Earth's position
glm::vec3 calculateMoonPosition(glm::vec3 earthPos, float angle) {
    float x = earthPos.x + MOON_ORBIT_RADIUS * cos(angle);
    float z = earthPos.z + MOON_ORBIT_RADIUS * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

// Returns the world position of Mars on its elliptical orbit given an angle
glm::vec3 calculateMarsPosition(float angle) {
    float x = MARS_ORBIT_SEMI_MAJOR * cos(angle);
    float z = MARS_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

// Checks if a solar eclipse is occurring (Moon between Sun and Earth)
// Uses dot product alignment and triangle inequality to verify ordering
bool checkSolarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 sunToMoon = moonPos - sunPos;
    
    // Normalize to get direction vectors for angle comparison
    sunToEarth = glm::normalize(sunToEarth);
    sunToMoon = glm::normalize(sunToMoon);
    
    // Dot product close to 1.0 means nearly parallel directions (aligned)
    float alignment = glm::dot(sunToEarth, sunToMoon);
    
    float sunToMoonDist = glm::length(moonPos - sunPos);
    float moonToEarthDist = glm::length(earthPos - moonPos);
    float sunToEarthDist = glm::length(earthPos - sunPos);
    
    // Check 1: directions are nearly identical (threshold: 0.9995)
    bool isAligned = alignment > 0.9995f;
    // Check 2: Moon is between Sun and Earth (triangle inequality)
    bool moonBetween = (sunToMoonDist + moonToEarthDist) < (sunToEarthDist * 1.01f);
    // Check 3: all three bodies are on the same orbital plane (Y ~ 0)
    bool samePlane = std::abs(sunPos.y - earthPos.y) < 0.1f && std::abs(earthPos.y - moonPos.y) < 0.1f;
    
    if (isAligned && moonBetween && samePlane) {
        return true;
    }
    
    return false;
}

// Checks if a lunar eclipse is occurring (Earth between Sun and Moon)
// Uses the same dot product and ordering logic as solar eclipse detection
bool checkLunarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 earthToMoon = moonPos - earthPos;
    
    // Normalize direction vectors
    sunToEarth = glm::normalize(sunToEarth);
    earthToMoon = glm::normalize(earthToMoon);
    
    // Dot product close to 1.0 means Sun, Earth, Moon are aligned in that order
    float alignment = glm::dot(sunToEarth, earthToMoon);
    
    float sunToEarthDist = glm::length(earthPos - sunPos);
    float earthToMoonDist = glm::length(moonPos - earthPos);
    float sunToMoonDist = glm::length(moonPos - sunPos);
    
    // Check 1: directions are nearly identical
    bool isAligned = alignment > 0.9995f;
    // Check 2: Earth is between Sun and Moon
    bool earthBetween = (sunToEarthDist + earthToMoonDist) < (sunToMoonDist * 1.01f);
    // Check 3: all bodies on the same plane
    bool samePlane = std::abs(sunPos.y - earthPos.y) < 0.1f && std::abs(earthPos.y - moonPos.y) < 0.1f;
    
    if (isAligned && earthBetween && samePlane) {
        return true;
    }
    
    return false;
}

// Processes all keyboard input each frame
void processInput(GLFWwindow* window) {
    // ESC closes the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement: W/A/S/D for forward/back/left/right, Space/Shift for up/down
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(3, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(4, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(5, deltaTime);

    // G: activate fast-forward to search for solar eclipse
    static bool gKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gKeyPressed) {
        gKeyPressed = true;
        if (!isEclipse && !isLunarEclipse) {
            speedUpMode = true;
            speedUpModeLunar = false;
            timeSpeed = fastTimeSpeed;
            std::cout << "Speed up mode activated! Searching for solar eclipse..." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        gKeyPressed = false;
    }

    // H: activate fast-forward to search for lunar eclipse
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        if (!isLunarEclipse && !isEclipse) {
            speedUpModeLunar = true;
            speedUpMode = false;
            timeSpeed = fastTimeSpeed;
            std::cout << "Lunar speed up mode activated! Searching for lunar eclipse..." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        hKeyPressed = false;
    }

    // J: resume normal simulation after an eclipse
    static bool jKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !jKeyPressed) {
        jKeyPressed = true;
        if (isEclipse) {
            isEclipse = false;
            speedUpMode = false;
            timeSpeed = normalTimeSpeed;
            moonPosAdjusted = false;
            std::cout << "Solar eclipse ended. Normal movement resumed. Press G to search for eclipse again." << std::endl;
        } else if (isLunarEclipse) {
            isLunarEclipse = false;
            speedUpModeLunar = false;
            timeSpeed = normalTimeSpeed;
            moonPosAdjusted = false;
            std::cout << "Lunar eclipse ended. Normal movement resumed. Press H to search for eclipse again." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
        jKeyPressed = false;
    }

    // V: toggle camera follow mode (track Earth's position)
    static bool vKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vKeyPressed) {
        vKeyPressed = true;
        cameraFollowEarth = !cameraFollowEarth;
        if (cameraFollowEarth) {
            std::cout << "Camera now following Earth. Press V again to return to free camera." << std::endl;
        } else {
            std::cout << "Camera returned to free mode." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        vKeyPressed = false;
    }

    // R: full reset - clears eclipse states and restores normal speed
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
        rKeyPressed = true;
        isEclipse = false;
        isLunarEclipse = false;
        speedUpMode = false;
        speedUpModeLunar = false;
        timeSpeed = normalTimeSpeed;
        cameraFollowEarth = false;
        moonPosAdjusted = false;
        std::cout << "Reset. Normal speed resumed." << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        rKeyPressed = false;
    }
}

// Called by GLFW when the window is resized — updates the OpenGL viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Called by GLFW on mouse movement — updates camera yaw and pitch
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // On first mouse event, initialize last position to avoid a large initial jump
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate mouse offset since last frame (Y is inverted: up = negative screen Y)
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Called by GLFW on mouse scroll — adjusts camera zoom (FOV)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}