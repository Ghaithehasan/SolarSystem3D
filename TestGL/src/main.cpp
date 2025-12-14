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

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 30.0f, 80.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float earthOrbitAngle = 0.0f;
float moonOrbitAngle = 0.0f;
float marsOrbitAngle = 0.0f;
float earthRotationAngle = 0.0f;
float timeSpeed = 0.5f;
float normalTimeSpeed = 0.5f;
float fastTimeSpeed = 4.0f;
bool isEclipse = false;
bool isLunarEclipse = false;
bool speedUpMode = false;
bool speedUpModeLunar = false;
bool cameraFollowEarth = false;
glm::vec3 adjustedMoonPos;
bool moonPosAdjusted = false;

const float SUN_RADIUS = 10.0f;
const float EARTH_RADIUS = 3.0f;
const float MOON_RADIUS = 2.4f;
const float MARS_RADIUS = 1.5f;

const float EARTH_ORBIT_SEMI_MAJOR = 60.0f;
const float EARTH_ORBIT_SEMI_MINOR = 55.0f;
const float MOON_ORBIT_RADIUS = 12.0f;
const float MARS_ORBIT_SEMI_MAJOR = 85.0f;
const float MARS_ORBIT_SEMI_MINOR = 80.0f;

glm::vec3 sunColor(1.0f, 0.95f, 0.8f);
glm::vec3 earthColor(0.15f, 0.5f, 0.7f);
glm::vec3 moonColor(0.75f, 0.75f, 0.8f);
glm::vec3 marsColor(0.8f, 0.3f, 0.2f);

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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader solarShader("shaders/solar_vertex.glsl", "shaders/solar_fragment.glsl");
    Shader skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
    Shader orbitShader("shaders/orbit_vertex.glsl", "shaders/orbit_fragment.glsl");

    unsigned int sunTexture = TextureLoader::loadTexture("textures/8k_sun.jpg", false);
    unsigned int earthDayTexture = TextureLoader::loadTexture("textures/2k_earth_daymap.jpg", false);
    unsigned int earthNightTexture = TextureLoader::loadTexture("textures/2k_earth_nightmap.jpg", false);
    unsigned int earthCloudsTexture = TextureLoader::loadTexture("textures/2k_earth_clouds.jpg", false);
    unsigned int moonTexture = TextureLoader::loadTexture("textures/2k_moon.jpg", false);
    unsigned int marsTexture = TextureLoader::loadTexture("textures/8k_mars.jpg", false);

    Skybox skybox;
    skybox.loadTexture("textures/2k_stars_milky_way.jpg");

    OrbitPath earthOrbitPath;
    earthOrbitPath.generateEarthOrbit(EARTH_ORBIT_SEMI_MAJOR, EARTH_ORBIT_SEMI_MINOR, 120);
    
    OrbitPath moonOrbitPath;
    moonOrbitPath.generateMoonOrbit(MOON_ORBIT_RADIUS, 80);

    Sphere sun(SUN_RADIUS, 50, 50);
    Sphere earth(EARTH_RADIUS, 40, 40);
    Sphere moon(MOON_RADIUS, 30, 30);
    Sphere mars(MARS_RADIUS, 35, 35);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (!isEclipse && !isLunarEclipse) {
            float clampedDeltaTime = std::min(deltaTime, 0.1f);
            earthOrbitAngle += 0.3f * timeSpeed * clampedDeltaTime;
            moonOrbitAngle += 1.2f * timeSpeed * clampedDeltaTime;
            marsOrbitAngle += 0.15f * timeSpeed * clampedDeltaTime;
            earthRotationAngle += 2.0f * timeSpeed * clampedDeltaTime;
        }

        glm::vec3 sunPos(0.0f, 0.0f, 0.0f);
        glm::vec3 earthPos = calculateEarthPosition(earthOrbitAngle);
        glm::vec3 moonPos;
        if (moonPosAdjusted && (isEclipse || isLunarEclipse)) {
            moonPos = adjustedMoonPos;
        } else {
            moonPos = calculateMoonPosition(earthPos, moonOrbitAngle);
            moonPosAdjusted = false;
        }
        glm::vec3 marsPos = calculateMarsPosition(marsOrbitAngle);

        if (cameraFollowEarth) {
            glm::vec3 lookTarget;
            if (isEclipse) {
                lookTarget = moonPos;
            } else {
                lookTarget = sunPos;
            }
            
            glm::vec3 direction = glm::normalize(lookTarget - earthPos);
            
            glm::vec3 cameraPos = earthPos - direction * EARTH_RADIUS;
            
            camera.SetPositionAndLookAt(cameraPos, lookTarget);
        }

        if (speedUpMode && !isEclipse && !isLunarEclipse) {
            isEclipse = checkSolarEclipse(sunPos, earthPos, moonPos);
            if (isEclipse) {
                glm::vec3 sunToEarth = earthPos - sunPos;
                float sunToEarthDist = glm::length(sunToEarth);
                glm::vec3 sunToEarthDir = glm::normalize(sunToEarth);
                
                float moonDistFromEarth = MOON_ORBIT_RADIUS;
                float moonDistFromSun = sunToEarthDist - moonDistFromEarth;
                
                if (moonDistFromSun > 0 && moonDistFromSun < sunToEarthDist) {
                    adjustedMoonPos = sunPos + sunToEarthDir * moonDistFromSun;
                    adjustedMoonPos.y = 0.0f;
                    moonPosAdjusted = true;
                    
                    glm::vec3 verifySunToMoon = adjustedMoonPos - sunPos;
                    glm::vec3 verifyMoonToEarth = earthPos - adjustedMoonPos;
                    float verifyAlignment = glm::dot(glm::normalize(verifySunToMoon), glm::normalize(verifyMoonToEarth));
                    
                    std::cout << "SOLAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                    std::cout << "Alignment verification: " << verifyAlignment << " (should be ~1.0)" << std::endl;
                }
                
                timeSpeed = 0.0f;
            }
        }

        if (speedUpModeLunar && !isLunarEclipse && !isEclipse) {
            isLunarEclipse = checkLunarEclipse(sunPos, earthPos, moonPos);
            if (isLunarEclipse) {
                glm::vec3 sunToEarth = glm::normalize(earthPos - sunPos);
                float earthToMoonDist = MOON_ORBIT_RADIUS;
                moonPos = earthPos + sunToEarth * earthToMoonDist;
                moonPos.y = 0.0f;
                
                std::cout << "LUNAR ECLIPSE DETECTED! Movement stopped. Perfect alignment achieved." << std::endl;
                timeSpeed = 0.0f;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        skyboxShader.setInt("skybox", 0);
        skybox.Draw();
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.5f);
        orbitShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        orbitShader.setMat4("projection", projection);
        orbitShader.setMat4("view", view);
        
        glm::mat4 model = glm::mat4(1.0f);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.8f, 0.8f, 0.9f));
        earthOrbitPath.Draw();
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
        orbitShader.setMat4("model", model);
        orbitShader.setVec3("orbitColor", glm::vec3(0.7f, 0.7f, 0.8f));
        moonOrbitPath.Draw();
        
        glDisable(GL_BLEND);
        glLineWidth(1.0f);

        solarShader.use();

        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        view = camera.GetViewMatrix();
        solarShader.setMat4("projection", projection);
        solarShader.setMat4("view", view);
        solarShader.setVec3("viewPos", camera.Position);

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
            
        model = glm::mat4(1.0f);
        model = glm::translate(model, earthPos);
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

        model = glm::mat4(1.0f);
        model = glm::translate(model, marsPos);
        solarShader.setMat4("model", model);
        solarShader.setVec3("objectColor", marsColor);
        solarShader.setInt("objectType", 1);
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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

glm::vec3 calculateEarthPosition(float angle) {
    float x = EARTH_ORBIT_SEMI_MAJOR * cos(angle);
    float z = EARTH_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

glm::vec3 calculateMoonPosition(glm::vec3 earthPos, float angle) {
    float x = earthPos.x + MOON_ORBIT_RADIUS * cos(angle);
    float z = earthPos.z + MOON_ORBIT_RADIUS * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

glm::vec3 calculateMarsPosition(float angle) {
    float x = MARS_ORBIT_SEMI_MAJOR * cos(angle);
    float z = MARS_ORBIT_SEMI_MINOR * sin(angle);
    return glm::vec3(x, 0.0f, z);
}

bool checkSolarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 sunToMoon = moonPos - sunPos;
    
    sunToEarth = glm::normalize(sunToEarth);
    sunToMoon = glm::normalize(sunToMoon);
    
    float alignment = glm::dot(sunToEarth, sunToMoon);
    
    float sunToMoonDist = glm::length(moonPos - sunPos);
    float moonToEarthDist = glm::length(earthPos - moonPos);
    float sunToEarthDist = glm::length(earthPos - sunPos);
    
    bool isAligned = alignment > 0.9995f;
    bool moonBetween = (sunToMoonDist + moonToEarthDist) < (sunToEarthDist * 1.01f);
    bool samePlane = std::abs(sunPos.y - earthPos.y) < 0.1f && std::abs(earthPos.y - moonPos.y) < 0.1f;
    
    if (isAligned && moonBetween && samePlane) {
        return true;
    }
    
    return false;
}

bool checkLunarEclipse(glm::vec3 sunPos, glm::vec3 earthPos, glm::vec3 moonPos) {
    glm::vec3 sunToEarth = earthPos - sunPos;
    glm::vec3 earthToMoon = moonPos - earthPos;
    
    sunToEarth = glm::normalize(sunToEarth);
    earthToMoon = glm::normalize(earthToMoon);
    
    float alignment = glm::dot(sunToEarth, earthToMoon);
    
    float sunToEarthDist = glm::length(earthPos - sunPos);
    float earthToMoonDist = glm::length(moonPos - earthPos);
    float sunToMoonDist = glm::length(moonPos - sunPos);
    
    bool isAligned = alignment > 0.9995f;
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
