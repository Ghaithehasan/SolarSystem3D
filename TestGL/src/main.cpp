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

// Window dimensions
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera
Camera camera(glm::vec3(0.0f, 30.0f, 80.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Orbital parameters
float earthOrbitAngle = 0.0f;
float moonOrbitAngle = 0.0f;
float marsOrbitAngle = 0.0f;
float earthRotationAngle = 0.0f;  // Earth's rotation around its axis
float timeSpeed = 0.5f;  // Faster normal speed
float normalTimeSpeed = 0.5f;  // Faster normal speed
float fastTimeSpeed = 4.0f;  // Fast speed for eclipse search
bool isEclipse = false;  // Solar eclipse
bool isLunarEclipse = false;  // Lunar eclipse
bool speedUpMode = false;  // For solar eclipse search
bool speedUpModeLunar = false;  // For lunar eclipse search
bool cameraFollowEarth = false;  // Camera follows Earth when V is pressed
glm::vec3 adjustedMoonPos;  // Adjusted Moon position during eclipse
bool moonPosAdjusted = false;  // Flag to use adjusted position

// Sizes (adjusted so Moon covers Sun during eclipse when viewed from Earth)
// In reality: Sun diameter ~400x Moon, but Sun is ~400x farther, so they appear same size
// For eclipse visualization: Moon should appear slightly larger than Sun from Earth
const float SUN_RADIUS = 10.0f;
const float EARTH_RADIUS = 3.0f;
const float MOON_RADIUS = 2.4f;  // Larger moon, slightly smaller than Earth
const float MARS_RADIUS = 1.5f;  // Half the size of Earth

// Distances (scaled for visualization)
const float EARTH_ORBIT_SEMI_MAJOR = 60.0f;  // Semi-major axis for elliptical orbit
const float EARTH_ORBIT_SEMI_MINOR = 55.0f;  // Semi-minor axis
const float MOON_ORBIT_RADIUS = 12.0f;       // Circular orbit around Earth
const float MARS_ORBIT_SEMI_MAJOR = 85.0f;  // Closer to Earth for better visualization
const float MARS_ORBIT_SEMI_MINOR = 80.0f;  // Closer to Earth for better visualization

// Colors
glm::vec3 sunColor(1.0f, 0.95f, 0.8f);      // Yellow-white
glm::vec3 earthColor(0.15f, 0.5f, 0.7f);    // Blue-green (mix of ocean blue and land green)
glm::vec3 moonColor(0.75f, 0.75f, 0.8f);    // Light gray
glm::vec3 marsColor(0.8f, 0.3f, 0.2f);     // Red-orange (Mars color)

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Orbital calculations
glm::vec3 calculateEarthPosition(float angle);
glm::vec3 calculateMoonPosition(glm::vec3 earthPos, float angle);
glm::vec3 calculateMarsPosition(float angle);
bool checkSolarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos);
bool checkLunarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos);

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System - Earth, Moon & Sun", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Build and compile shader programs
    Shader solarShader("shaders/solar_vertex.glsl", "shaders/solar_fragment.glsl");
    Shader skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
    Shader orbitShader("shaders/orbit_vertex.glsl", "shaders/orbit_fragment.glsl");

    // Load textures
    unsigned int sunTexture = TextureLoader::loadTexture("textures/8k_sun.jpg", false);
    unsigned int earthDayTexture = TextureLoader::loadTexture("textures/2k_earth_daymap.jpg", false);
    unsigned int earthNightTexture = TextureLoader::loadTexture("textures/2k_earth_nightmap.jpg", false);
    unsigned int earthCloudsTexture = TextureLoader::loadTexture("textures/2k_earth_clouds.jpg", false);
    unsigned int moonTexture = TextureLoader::loadTexture("textures/2k_moon.jpg", false);
    unsigned int marsTexture = TextureLoader::loadTexture("textures/8k_mars.jpg", false);

    // Create skybox
    Skybox skybox;
    skybox.loadTexture("textures/2k_stars_milky_way.jpg");

    // Create orbit paths
    OrbitPath earthOrbitPath;
    earthOrbitPath.generateEarthOrbit(EARTH_ORBIT_SEMI_MAJOR, EARTH_ORBIT_SEMI_MINOR, 120);
    
    OrbitPath moonOrbitPath;
    moonOrbitPath.generateMoonOrbit(MOON_ORBIT_RADIUS, 80);

    // Create spheres
    Sphere sun(SUN_RADIUS, 50, 50);
    Sphere earth(EARTH_RADIUS, 40, 40);
    Sphere moon(MOON_RADIUS, 30, 30);
    Sphere mars(MARS_RADIUS, 35, 35);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Update orbital positions
        if (!isEclipse && !isLunarEclipse) {
            // Clamp deltaTime to prevent crashes from very large values
            float clampedDeltaTime = std::min(deltaTime, 0.1f);
            earthOrbitAngle += 0.3f * timeSpeed * clampedDeltaTime;
            moonOrbitAngle += 1.2f * timeSpeed * clampedDeltaTime; // Moon orbits faster
            marsOrbitAngle += 0.15f * timeSpeed * clampedDeltaTime; // Mars orbits slower (farther from sun)
            earthRotationAngle += 2.0f * timeSpeed * clampedDeltaTime; // Earth rotates around its axis
        }

        // Calculate positions
        glm::vec3 sunPos(0.0f, 0.0f, 0.0f);
        glm::vec3 earthPos = calculateEarthPosition(earthOrbitAngle);
        glm::vec3 moonPos;
        if (moonPosAdjusted && (isEclipse || isLunarEclipse)) {
            // Use adjusted Moon position during eclipse
            moonPos = adjustedMoonPos;
        } else {
            // Calculate normal Moon position
            moonPos = calculateMoonPosition(earthPos, moonOrbitAngle);
            moonPosAdjusted = false;  // Reset flag when not in eclipse
        }
        glm::vec3 marsPos = calculateMarsPosition(marsOrbitAngle);

        // Update camera position if following Earth
        if (cameraFollowEarth) {
            // Calculate direction from Earth to Sun/Moon
            glm::vec3 lookTarget;
            if (isEclipse) {
                // During solar eclipse, look at Moon (which covers Sun)
                lookTarget = moonPos;
            } else {
                // Otherwise, look at Sun
                lookTarget = sunPos;
            }
            
            // Calculate direction vector from Earth center to target
            glm::vec3 direction = glm::normalize(lookTarget - earthPos);
            
            // Position camera on Earth's surface, in the direction opposite to the target
            // This places the camera on the surface looking up at the sky
            glm::vec3 cameraPos = earthPos - direction * EARTH_RADIUS;
            
            // Adjust camera to look directly at the target
            camera.SetPositionAndLookAt(cameraPos, lookTarget);
        }

        // Check for solar eclipse (only when in speed up mode)
        if (speedUpMode && !isEclipse && !isLunarEclipse) {
            isEclipse = checkSolarEclipse(sunPos, earthPos, moonPos);
            if (isEclipse) {
                // Adjust Moon position to be perfectly aligned on the line Sun -> Moon -> Earth
                glm::vec3 sunToEarth = earthPos - sunPos;
                float sunToEarthDist = glm::length(sunToEarth);
                glm::vec3 sunToEarthDir = glm::normalize(sunToEarth);
                
                // Place Moon exactly on the line between Sun and Earth
                // Moon should be at a distance from Earth equal to MOON_ORBIT_RADIUS
                float moonDistFromEarth = MOON_ORBIT_RADIUS;
                float moonDistFromSun = sunToEarthDist - moonDistFromEarth;
                
                // Ensure Moon is between Sun and Earth
                if (moonDistFromSun > 0 && moonDistFromSun < sunToEarthDist) {
                    adjustedMoonPos = sunPos + sunToEarthDir * moonDistFromSun;
                    adjustedMoonPos.y = 0.0f;  // Ensure same plane (y=0)
                    moonPosAdjusted = true;
                    
                    // Verify perfect alignment
                    glm::vec3 verifySunToMoon = adjustedMoonPos - sunPos;
                    glm::vec3 verifyMoonToEarth = earthPos - adjustedMoonPos;
                    float verifyAlignment = glm::dot(glm::normalize(verifySunToMoon), glm::normalize(verifyMoonToEarth));
                    
                    std::cout << "SOLAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                    std::cout << "Alignment verification: " << verifyAlignment << " (should be ~1.0)" << std::endl;
                }
                
                timeSpeed = 0.0f;  // Stop movement completely
            }
        }

        // Check for lunar eclipse (only when in lunar speed up mode)
        if (speedUpModeLunar && !isLunarEclipse && !isEclipse) {
            isLunarEclipse = checkLunarEclipse(sunPos, earthPos, moonPos);
            if (isLunarEclipse) {
                // Adjust Moon position to be perfectly aligned
                glm::vec3 sunToEarth = glm::normalize(earthPos - sunPos);
                // Place Moon on the line beyond Earth, opposite to Sun
                float earthToMoonDist = MOON_ORBIT_RADIUS;
                moonPos = earthPos + sunToEarth * earthToMoonDist;
                moonPos.y = 0.0f;  // Ensure same plane
                
                std::cout << "LUNAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                timeSpeed = 0.0f;  // Stop movement completely
            }
        }

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        skyboxShader.setInt("skybox", 0);
        skybox.Draw();
        glDepthFunc(GL_LESS);

        // Draw orbit paths
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.5f);
        orbitShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        orbitShader.setMat4("projection", projection);
        orbitShader.setMat4("view", view);
        
        // Draw Earth's orbit around Sun
        glm::mat4 model = glm::mat4(1.0f);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.8f, 0.8f, 0.9f));  // Light blue-white
        earthOrbitPath.Draw();
        
        // Draw Moon's orbit around Earth (translate to Earth's position)
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.7f, 0.7f, 0.8f));  // Slightly darker
        moonOrbitPath.Draw();
        
        glDisable(GL_BLEND);
        glLineWidth(1.0f);

        // Activate solar shader
        solarShader.use();

        // Projection and view matrices
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        solarShader.setMat4("projection", projection);
        solarShader.setMat4("view", view);
        solarShader.setVec3("viewPos", camera.Position);

        // Draw Sun
        model = glm::mat4(1.0f);
        model = glm::translate(model, sunPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", sunColor);
        solarShader.setInt("objectType", 0);
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

        // Draw Earth
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
        // Rotate Earth around its axis (Y-axis for vertical rotation)
        model = glm::rotate(model, earthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", earthColor);
        solarShader.setInt("objectType", 1);
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", false);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", true);
        solarShader.setBool("useCloudsTexture", true);
        
        // Earth day texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthDayTexture);
        solarShader.setInt("diffuseTexture", 0);
        
        // Earth night texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earthNightTexture);
        solarShader.setInt("nightTexture", 1);
        
        // Earth clouds texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earthCloudsTexture);
        solarShader.setInt("cloudsTexture", 2);
        
        earth.Draw();

        // Draw Moon
        model = glm::mat4(1.0f);
        model = glm::translate(model, moonPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", moonColor);
        solarShader.setInt("objectType", 2);
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", true);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", false);
        solarShader.setBool("useCloudsTexture", false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        solarShader.setInt("diffuseTexture", 0);
        moon.Draw();

        // Draw Mars
        model = glm::mat4(1.0f);
        model = glm::translate(model, marsPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", marsColor);
        solarShader.setInt("objectType", 1);  // Use same type as Earth for lighting
        solarShader.setVec3("sunPos", sunPos);
        solarShader.setVec3("sunColor", sunColor);
        solarShader.setFloat("sunIntensity", 2.0f);
        solarShader.setVec3("moonPos", moonPos);
        solarShader.setVec3("moonColor", glm::vec3(0.9f, 0.9f, 0.95f));
        solarShader.setFloat("moonIntensity", 0.3f);
        solarShader.setBool("isMoon", false);
        solarShader.setBool("useTexture", true);
        solarShader.setBool("useNightTexture", false);
        solarShader.setBool("useCloudsTexture", false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, marsTexture);
        solarShader.setInt("diffuseTexture", 0);
        mars.Draw();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

glm::vec3 calculateEarthPosition(float angle) {
    // Elliptical orbit
    float x = EARTH_ORBIT_SEMI_MAJOR * cos(angle);
    float z = EARTH_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

glm::vec3 calculateMoonPosition(glm::vec3 earthPos, float angle) {
    // Circular orbit around Earth
    float x = earthPos.x + MOON_ORBIT_RADIUS * cos(angle);
    float z = earthPos.z + MOON_ORBIT_RADIUS * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

glm::vec3 calculateMarsPosition(float angle) {
    // Elliptical orbit (double the distance of Earth)
    float x = MARS_ORBIT_SEMI_MAJOR * cos(angle);
    float z = MARS_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

bool checkSolarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    // Check if Sun, Moon, and Earth are perfectly aligned (180 degrees - straight line)
    // Sun -> Moon -> Earth should form a straight line
    
    // Calculate vectors
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 sunToMoon = moonPos - sunPos;
    
    // Normalize vectors
    sunToEarth = glm::normalize(sunToEarth);
    sunToMoon = glm::normalize(sunToMoon);
    
    // For perfect 180 degree alignment, sunToEarth and sunToMoon should point in the same direction
    // Dot product of 1.0 means perfect alignment
    float alignment = glm::dot(sunToEarth, sunToMoon);
    
    // Check if Moon is between Sun and Earth (distance check)
    float sunToMoonDist = glm::length(moonPos - sunPos);
    float moonToEarthDist = glm::length(earthPos - moonPos);
    float sunToEarthDist = glm::length(earthPos - sunPos);
    
    // Eclipse occurs when:
    // 1. Perfect alignment (very close to 1.0 for 180 degrees)
    // 2. Moon is between Sun and Earth (distance check)
    // 3. All three are in the same plane (check Y coordinates are close)
    bool isAligned = alignment > 0.9995f;  // Very close to perfect alignment
    bool moonBetween = (sunToMoonDist + moonToEarthDist) < (sunToEarthDist * 1.01f);
    bool samePlane = std::abs(sunPos.y - earthPos.y) < 0.1f && std::abs(earthPos.y - moonPos.y) < 0.1f;
    
    if (isAligned && moonBetween && samePlane) {
        return true;
    }
    
    return false;
}

bool checkLunarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    // Check if Sun, Earth, and Moon are perfectly aligned (180 degrees - straight line)
    // Sun -> Earth -> Moon should form a straight line (Earth between Sun and Moon)
    
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 earthToMoon = moonPos - earthPos;
    
    // Normalize vectors
    sunToEarth = glm::normalize(sunToEarth);
    earthToMoon = glm::normalize(earthToMoon);
    
    // For perfect 180 degree alignment, the vectors should point in the same direction
    // Dot product of 1.0 means perfect alignment (180 degrees)
    float alignment = glm::dot(sunToEarth, earthToMoon);
    
    // Check if Earth is between Sun and Moon (distance check)
    float sunToEarthDist = glm::length(earthPos - sunPos);
    float earthToMoonDist = glm::length(moonPos - earthPos);
    float sunToMoonDist = glm::length(moonPos - sunPos);
    
    // Lunar eclipse occurs when:
    // 1. Perfect alignment (very close to 1.0 for 180 degrees)
    // 2. Earth is between Sun and Moon (distance check)
    // 3. All three are in the same plane (check Y coordinates are close)
    bool isAligned = alignment > 0.9995f;  // Very close to perfect alignment
    bool earthBetween = (sunToEarthDist + earthToMoonDist) < (sunToMoonDist * 1.01f);
    bool samePlane = std::abs(sunPos.y - earthPos.y) < 0.1f && std::abs(earthPos.y - moonPos.y) < 0.1f;
    
    if (isAligned && earthBetween && samePlane) {
        return true;
    }
    
    return false;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
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

    // Speed up to solar eclipse (G key) - activate speed up mode
    static bool gKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gKeyPressed) {
        gKeyPressed = true;
        if (!isEclipse && !isLunarEclipse) {
            // Always activate speed up mode when G is pressed (if not in any eclipse)
            speedUpMode = true;
            speedUpModeLunar = false;  // Disable lunar mode
            timeSpeed = fastTimeSpeed;
            std::cout << "Speed up mode activated! Searching for solar eclipse..." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        gKeyPressed = false;
    }

    // Speed up to lunar eclipse (H key) - activate lunar speed up mode
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        if (!isLunarEclipse && !isEclipse) {
            // Always activate lunar speed up mode when H is pressed (if not in any eclipse)
            speedUpModeLunar = true;
            speedUpMode = false;  // Disable solar mode
            timeSpeed = fastTimeSpeed;
            std::cout << "Lunar speed up mode activated! Searching for lunar eclipse..." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        hKeyPressed = false;
    }

    // Resume normal movement after solar eclipse (J key)
    static bool jKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !jKeyPressed) {
        jKeyPressed = true;
        if (isEclipse) {
            // Resume normal movement and reset speed up mode
            isEclipse = false;
            speedUpMode = false;  // Reset speed up mode so G can be used again
            timeSpeed = normalTimeSpeed;
            moonPosAdjusted = false;  // Reset Moon position adjustment
            std::cout << "Solar eclipse ended. Normal movement resumed. Press G to search for eclipse again." << std::endl;
        } else if (isLunarEclipse) {
            // Resume normal movement and reset lunar speed up mode
            isLunarEclipse = false;
            speedUpModeLunar = false;  // Reset speed up mode so H can be used again
            timeSpeed = normalTimeSpeed;
            moonPosAdjusted = false;  // Reset Moon position adjustment
            std::cout << "Lunar eclipse ended. Normal movement resumed. Press H to search for eclipse again." << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
        jKeyPressed = false;
    }

    // Toggle camera follow Earth (V key)
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

    // Reset eclipse (R key) - emergency reset
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
        rKeyPressed = true;
        isEclipse = false;
        isLunarEclipse = false;
        speedUpMode = false;
        speedUpModeLunar = false;
        timeSpeed = normalTimeSpeed;
        cameraFollowEarth = false;  // Also reset camera follow
        moonPosAdjusted = false;  // Reset Moon position adjustment
        std::cout << "Reset. Normal speed resumed." << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        rKeyPressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
