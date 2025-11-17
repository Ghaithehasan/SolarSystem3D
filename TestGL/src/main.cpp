#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "PendulumClock.h"

glm::vec3 target(0.0f, 0.0f, 0.0f);  // نقطة الهدف (الساعة)
float distance = 3.0f;  // المسافة من الكاميرا إلى الهدف
float azimuth = 0.0f;   // الزاوية الأفقية (بالراديان)
float elevation = 0.0f; // الزاوية العمودية (بالراديان)

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
bool rightMousePressed = false;  // حالة الضغط على زر الماوس الأيمن
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// دالة لتحديث موضع الكاميرا بناءً على Orbit Control
void updateCameraPosition() {
    // حساب موضع الكاميرا باستخدام الإحداثيات الكروية
    float x = target.x + distance * cos(elevation) * sin(azimuth);
    float y = target.y + distance * sin(elevation);
    float z = target.z + distance * cos(elevation) * cos(azimuth);
    
    camera.Position = glm::vec3(x, y, z);
    
    // تحديث اتجاه الكاميرا لتكون تنظر نحو الهدف
    glm::vec3 direction = glm::normalize(target - camera.Position);
    camera.Front = direction;
    
    // تحديث Yaw و Pitch للكاميرا
    camera.Yaw = glm::degrees(atan2(direction.x, direction.z));
    camera.Pitch = glm::degrees(asin(direction.y));
    
    // تحديث vectors الكاميرا يدوياً (لأن updateCameraVectors() private)
    camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
    camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
}

// دالة callback لتغيير حجم النافذة
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// دالة callback لحركة الماوس
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Orbit Control - التحكم بالكاميرا فقط عند الضغط على زر الماوس الأيمن
    if (!rightMousePressed) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    // Orbit Control: تدوير الكاميرا حول الهدف
    float sensitivity = 0.005f;  // حساسية الدوران
    
    azimuth -= xoffset * sensitivity;  // الدوران الأفقي
    elevation += yoffset * sensitivity;  // الدوران العمودي
    
    // تحديد الحد الأقصى للزاوية العمودية (منع الدوران الكامل)
    const float maxElevation = 1.5f;  // حوالي 85 درجة
    if (elevation > maxElevation) elevation = maxElevation;
    if (elevation < -maxElevation) elevation = -maxElevation;
    
    // تحديث موضع الكاميرا
    updateCameraPosition();
}

// دالة callback لضغطات الماوس
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // عند الضغط على زر الماوس الأيمن - تفعيل التحكم
            rightMousePressed = true;
            firstMouse = true;  // إعادة تعيين لتفادي القفزة الأولى
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported())
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        } else if (action == GLFW_RELEASE) {
            // عند رفع زر الماوس الأيمن - تعطيل التحكم
            rightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// دالة callback لعجلة الماوس - Zoom In/Out
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Orbit Control: تغيير المسافة (Zoom)
    float zoomSpeed = 0.5f;
    distance -= (float)yoffset * zoomSpeed;
    
    // تحديد الحد الأدنى والأقصى للمسافة
    if (distance < 1.0f) distance = 1.0f;
    if (distance > 10.0f) distance = 10.0f;
    
    // تحديث موضع الكاميرا
    updateCameraPosition();
}

// معالجة الإدخال من لوحة المفاتيح
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

int main() {

    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
        });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // =====================
    // 2. إنشاء النافذة
    // =====================
    GLFWwindow* window = glfwCreateWindow(800, 600, "Pendulum Clock - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // =====================
    // إعداد callbacks
    // =====================
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // إظهار المؤشر افتراضياً - سيتم إخفاؤه عند الضغط على زر الماوس الأيمن
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // =====================
    // 3. تهيئة OpenGL عبر GLAD
    // =====================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    std::cout << "========================================" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "W/A/S/D - Move" << std::endl;
    std::cout << "SPACE/CTRL - Up/Down" << std::endl;
    std::cout << "Right Mouse Button + Drag - Rotate View" << std::endl;
    std::cout << "Scroll - Zoom In/Out" << std::endl;
    std::cout << "ESC - Exit" << std::endl;
    std::cout << "========================================" << std::endl;

    // =====================
    // 4. تحميل Shaders
    // =====================
    Shader shader("shaders/basic_vertex.glsl", "shaders/basic_fragment.glsl");

    // =====================
    // 5. إنشاء الساعة
    // =====================
    PendulumClock clock;
    clock.SetTime(10, 30);  // 10:30
    clock.StartPendulum();
    
    // تهيئة Orbit Control
    updateCameraPosition();

    // =====================
    // 6. حلقة الرسم الرئيسية
    // =====================
    std::cout << "Rendering Pendulum Clock..." << std::endl;

    while (!glfwWindowShouldClose(window)) {
        // حساب deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // إعدادات الإسقاط والكاميرا
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // تحديث الساعة
        clock.Update(deltaTime);

        // رسم الساعة
        clock.Draw(shader, view, projection, camera.Position);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // =====================
    // 7. تنظيف الموارد
    // =====================
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Program terminated successfully!" << std::endl;
    return 0;
}
