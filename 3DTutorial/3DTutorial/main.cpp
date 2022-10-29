#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "Model.h"
#include "filesystem.h"

#include <time.h>
#include <iostream>

struct Material
{
    glm::vec4 ambient,
        diffuse,
        specular;
    float shininess;
};

struct DirLight
{
    glm::vec3 direction{ 0.0f, 1.0f, 0.0f },
        ambient{ 1.0f, 1.0f, 1.0f },
        diffuse{ 1.0f, 1.0f, 1.0f },
        specular{ 1.0f, 1.0f, 1.0f };
};

struct PointLight
{
    float linear = 0.09f, quadratic = 0.032f;

    glm::vec3 pos,
        diffuse{ 1.0f, 1.0f, 1.0f },
        specular{ 1.0f, 1.0f, 1.0f };
};

void init_glfw();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, float deltaTime);
int construct();
int start();
int update(float currentTime, float prevTime, float deltaTime);

// Settings.
const unsigned int width = 800, height = 600;
bool reset = true;

// Camera.
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
const float FOV = 45.0f, nearPlane = 0.1f, farPlane = 100.0f;
bool firstMouse = true;

// Lighting.
const float ambientStrength = 0.2f, specularStrength = 0.5f;
const int specularPow = 32;
DirLight dirLight;
PointLight PLights[4];

unsigned int VBO, mainVAO, lightVAO;
unsigned int vertCount;
Shader s1("./Resources/Shaders/vShader.txt", "./Resources/Shaders/fShader.txt"),
      s2("./Resources/Shaders/vShader2.txt", "./Resources/Shaders/vShader2.txt");
Model m1("./Resources/Models/backpack/backpack.obj");

glm::vec3 modelPos[10];
glm::vec3 modelScales[10];
glm::vec3 modelRots[10];
Material modelMaterials[10];

// Timing.
float lastX, lastY;

glm::vec3 up;
glm::vec3 cameraPos, cameraDirection, cameraRight, cameraUp, cameraFront;

GLFWwindow* window;

int main()
{
    std::vector<int> result;
    result.push_back(construct());
    if (result[0] != 0)
        return -1;

    while (reset)
    {
        reset = false;
        glfwSetWindowShouldClose(window, false);

        result.push_back(start());
        if (result[1] != 0)
            return -2;

        // render loop
        // -----------

        float prevTime = glfwGetTime();
        while (!glfwWindowShouldClose(window))
        {

            result.push_back(update(glfwGetTime(), prevTime, glfwGetTime() - prevTime));
            prevTime = glfwGetTime();
        }

        for (unsigned int i = 2; i < result.size(); i++)
        {
            if (result[i] != 0)
            {
                return -((int)i + 1);
            }
        }
    }

    return 0;
}

int construct()
{
    // greet the person so that they know that this has began.
    std::cout << "Greetings!\n";

    // glfw: initialize and configure
    // ------------------------------
    init_glfw();

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to lock the mouse.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state.
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    stbi_set_flip_vertically_on_load(true);

    return 0;
}

int start()
{


    // Build and compile our shader.

    srand(time(NULL));

    dirLight.direction = glm::normalize(glm::vec3(
        (rand() % 100) / 100.0f,
        (rand() % 100) / 100.0f,
        (rand() % 100) / 100.0f));

    dirLight.ambient = glm::vec3(
        (rand() % 100) / 1000.0f,
        (rand() % 100) / 1000.0f,
        (rand() % 100) / 1000.0f);

    dirLight.diffuse = glm::vec3(
        (rand() % 100) / 500.0f,
        (rand() % 100) / 500.0f,
        (rand() % 100) / 500.0f);

    dirLight.specular = glm::vec3(
        (rand() % 100) / 500.0f,
        (rand() % 100) / 500.0f,
        (rand() % 100) / 500.0f);

    // Point Lights.
    for (int i = 0; i < 4; i++)
    {
        PLights[i].pos = glm::vec3(
            ((rand() % 100) / 50.0f - 0.5f) * 10,
            ((rand() % 100) / 50.0f - 1.0f) * 10,
            ((rand() % 100) / 50.0f - 1.0f) * 10);

        PLights[i].diffuse = glm::vec3(
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f);

        PLights[i].specular = glm::vec3(
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f);
    }

    for (int i = 0; i < 10; i++)
    {
        modelPos[i] = glm::vec3(((rand() % 100) / 50.0f - 0.5f) * 5,
            ((rand() % 100) / 50.0f - 1.0f) * 5,
            ((rand() % 100) / 50.0f - 1.0f) * 5);

        modelScales[i] = glm::vec3(
            (rand() % 100) / 100.0f + 0.5f,
            (rand() % 100) / 100.0f + 0.5f,
            (rand() % 100) / 100.0f + 0.5f);

        modelRots[i] = glm::vec3((rand() % 100) / 50.0f - 0.5f,
            (rand() % 100) / 50.0f - 1.0f,
            (rand() % 100) / 50.0f - 1.0f);

        modelMaterials[i].shininess = (rand() % 32000) / 100.0f;
    }

    // if you want wireframe, remove the double forward slash on the next line.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    s1.use();
    // directional light
    s1.setVec3("dirLight.direction", dirLight.direction);
    s1.setVec3("dirLight.ambient", dirLight.ambient);
    s1.setVec3("dirLight.diffuse", dirLight.diffuse);
    s1.setVec3("dirLight.specular", dirLight.specular);
    // point light 1
    s1.setVec3("PLights[0].pos", PLights[0].pos);
    s1.setVec3("PLights[0].diffuse", PLights[0].diffuse);
    s1.setVec3("PLights[0].specular", PLights[0].specular);
    s1.setFloat("PLights[0].linear", PLights[0].linear);
    s1.setFloat("PLights[0].quadratic", PLights[0].quadratic);
    // point light 2
    s1.setVec3("PLights[1].pos", PLights[1].pos);
    s1.setVec3("PLights[1].diffuse", PLights[1].diffuse);
    s1.setVec3("PLights[1].specular", PLights[1].specular);
    s1.setFloat("PLights[1].linear", PLights[1].linear);
    s1.setFloat("PLights[1].quadratic", PLights[1].quadratic);
    // point light 3
    s1.setVec3("PLights[2].pos", PLights[2].pos);
    s1.setVec3("PLights[2].diffuse", PLights[2].diffuse);
    s1.setVec3("PLights[2].specular", PLights[2].specular);
    s1.setFloat("PLights[2].linear", PLights[2].linear);
    s1.setFloat("PLights[2].quadratic", PLights[2].quadratic);
    // point light 4
    s1.setVec3("PLights[3].pos", PLights[3].pos);
    s1.setVec3("PLights[3].diffuse", PLights[3].diffuse);
    s1.setVec3("PLights[3].specular", PLights[3].specular);
    s1.setFloat("PLights[3].linear", PLights[3].linear);
    s1.setFloat("PLights[3].quadratic", PLights[3].quadratic);

    return 0;
}

int update(float currentTime, float prevTime, float deltaTime)
{
    // Input.
    processInput(window, deltaTime);
    camera.updateCameraVectors();

    // Screen clearing.
    glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // View/projection transformations.
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();


    // Main object.
    // Activate the shader.
    s1.use();

    // Spotlight, in this case a flashlight.
    s1.setVec3("spotLight.pos", camera.Position);
    s1.setVec3("spotLight.direction", camera.Front);
    s1.setVec3("spotLight.diffuse", 0.25f, 0.25f, 0.25f);
    s1.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    s1.setFloat("spotLight.cutoff", glm::cos(glm::radians(12.5f)));
    s1.setFloat("spotLight.outerCutoff", glm::cos(glm::radians(17.5f)));
    s1.setFloat("spotLight.linear", 0.09f);
    s1.setFloat("spotLight.quadratic", 0.032f);

    // Uniforms.
    s1.setVec3("viewPos", camera.Position);
    s1.setMat4("projection", projection);
    s1.setMat4("view", view);

    // Draw stuff.
    glBindVertexArray(mainVAO);

    for (int i = 0; i < 10; i++)
    {
        s1.setFloat("material.shininess", modelMaterials[i].shininess);

        glm::mat4 modelTransform(1.0f);
        modelTransform = glm::translate(modelTransform, modelPos[i]);
        modelTransform = glm::rotate(modelTransform, currentTime * glm::radians(50.0f), modelRots[i]);
        modelTransform = glm::scale(modelTransform, modelScales[i]);

        s1.setMat4("model", modelTransform);

        // Draw call.
        glDrawArrays(GL_TRIANGLES, 0, vertCount);
    }

    // Light cube.
    // Activate shader.
    s2.use();

    // Uniforms.
    s2.setMat4("projection", projection);
    s2.setMat4("view", view);
    glBindVertexArray(lightVAO);
    for (int i = 0; i < 4; i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, PLights[i].pos);

        s2.setMat4("model", model);

        s2.setVec4("col", PLights[i].diffuse.r, PLights[i].diffuse.g, PLights[i].diffuse.b, 1.0f);

        // Draw stuff.
        glDrawArrays(GL_TRIANGLES, 0, vertCount);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();

    std::cout << camera.Position.x << ' ' << camera.Position.y << ' ' << camera.Position.z << '\n';

    return 0;
}

void end()
{
    // Optional.

    // Delete buffers.
    glDeleteVertexArrays(1, &mainVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    // Neccesary.

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

}

void init_glfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        reset = true;
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed as y coords go from down to up.

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
/*#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("Resources/shaders/vShader.txt", "Resources/shaders/fShader.txt");

    // load models
    // -----------
    Model ourModel("Resources/Models/backpack/backpack.obj");


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}*/