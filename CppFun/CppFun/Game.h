#include "CustomComponents.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_focus_callback(GLFWwindow* window, int focused);
void ProcessInputs(GLFWwindow* window);

GLFWwindow* window;
ObjectWorld objectWorld;

Shader defaultShader;
ShaderInstance defaultShaderInst1(defaultShader);
float colors1[] = { 0.2f, 0.3f, 0.4f, 1.0f };
ShaderInstance defaultShaderInst2(defaultShader);
float colors2[] = { 0.5f, 0.6f, 0.7f, 1.0f };

system_clock::time_point computerStartTime;

void Start()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD.\n");
        return;
    }

    // Set a bunch of standard stuff.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);

    glViewport(0, 0, 800, 600);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,  // top left 
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f   // bottom right
    };
    uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };


    float vertices2[] = {
       -0.75f, -0.5f, 0.0f,  // bottom left
         0.0f, 0.75f, 0.0f,  // top middle 
        0.75f, -0.5f, 0.0f   // bottom right
    };
    defaultShader = Shader(const_cast<char*>(defaultVertexShader), const_cast<char*>(defaultFragmentShader));
    defaultShaderInst1.shader = defaultShader;
    defaultShaderInst2.shader = defaultShader;

    Shadf color1 = Shadf(const_cast<char*>("color\0"), colors1);
    defaultShaderInst1.shad4f.Add(color1);

    Shadf color2 = Shadf(const_cast<char*>("color\0"), colors2);
    defaultShaderInst2.shad4f.Add(color2);

    objectWorld = ObjectWorld();

    Object* triangle = new Object();
    triangle->components.Add(new Transform(triangle));
    triangle->components.Add(new Mesh(triangle, vertices2, 3, &defaultShaderInst2));
    objectWorld.AddObject(triangle);

    Object* square = new Object();
    square->components.Add(new Transform(square));
    square->components.Add(new Mesh2(square, vertices, 4, indices, 2, &defaultShaderInst1));
    objectWorld.AddObject(square);

    objectWorld.Start();
}

void Update(float deltaTime)
{
    std::string lowCPUModeDialogue;
    if (lowCPUMode)
        lowCPUModeDialogue = ", and low CPU mode is on.";
    else
        lowCPUModeDialogue = ", and low CPU mode is off.";

    glfwSetWindowTitle(window, ("C++ game, also, FPS ~= " + std::to_string((int)roundf(1.0f / deltaTime)) + lowCPUModeDialogue).c_str());
    
    ProcessInputs(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    objectWorld.Update(deltaTime);

    defaultShaderInst1.shad4f.ptr[0].value[0] = fmodf(defaultShaderInst1.shad4f.ptr[0].value[0] + deltaTime, 1.0f);
    defaultShaderInst2.shad4f.ptr[0].value[2] = fmodf(defaultShaderInst2.shad4f.ptr[0].value[2] + deltaTime, 1.0f);

    objectWorld.RenderUpdate(deltaTime);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void End()
{
    defaultShader.End();
    defaultShaderInst1.End();
    defaultShaderInst2.End();


    objectWorld.End();

    glfwTerminate();
}

void Run()
{
    Start();

    float desiredTimeBetweenFrames = 1.0f / 200.0f;
    computerStartTime = system_clock::now();
    float startTime = glfwGetTime();
    float lastTime = glfwGetTime();
    float tLastTime = lastTime;
    uint frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        float deltaTime;
        if (glfwGetTime() - lastTime >= desiredTimeBetweenFrames)
        {
            frameCount++;
            deltaTime = glfwGetTime() - tLastTime;
            tLastTime = glfwGetTime();
            //printf("%f, %f, ", glfwGetTime(), lastTime);
            lastTime = fmaxf(lastTime + desiredTimeBetweenFrames, tLastTime - 1.0f);
            //printf("%f\n", lastTime);
            Update(deltaTime);
            if (lowCPUMode && tLastTime - lastTime < desiredTimeBetweenFrames)
                sleep_for(microseconds(lroundf((tLastTime - lastTime) * 1000000)));
        }
    }

    printf("%f(time application has been running for) ~= %f(expected time the application has been running for)\n", glfwGetTime() - startTime, (float)frameCount * desiredTimeBetweenFrames);

    End();

    return;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void window_focus_callback(GLFWwindow* window, int focused)
{
    if (lowCPUModeBehavior == 1)
        lowCPUMode = focused == 0;
    else
        lowCPUMode = lowCPUModeBehavior == 2;
}

void ProcessInputs(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        for (int x = 0; x < 10000; x++)
            putchar('\n');
}