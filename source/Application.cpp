#include "Application.h"
#include "ApplicationEvent.h"
#include "Utilities.h"
//\------------------------------------------------------------------------------------------
//\ The main Appliction - Initialise the Applications window using the glfw library and 
//\------------------------------------------------------------------------------------------

#include "ShaderUtil.h"
#include "Dispatcher.h"

// Include OpenGL Header
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

// Include iostream for console logging
#include <iostream>

bool Application::Create(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool a_fullscreen)
{
    //Initialise GLFW 
    if (!glfwInit()) { return false; }

    m_windowWidth = a_windowWidth;
    m_windowHeight = a_windowHeight;

    //create a windowed mode window and it's OpenGL context 
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, a_applicationName,
        (a_fullscreen ? glfwGetPrimaryMonitor() : nullptr), nullptr);

    if (!m_window)
    {
        glfwTerminate();
        return false;
    }

    //make the window's context current 
    glfwMakeContextCurrent(m_window);

    // Initialise GLAD - Load in GL Extensions
    if (!gladLoadGL()) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }

    int major = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
    int minor = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
    int revision = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_REVISION);

    std::cout << "OpenGL Version " << major << "." << minor << "." << revision << std::endl;

    // Set up glfw window resize callback function
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow*, int w, int h)
    {
        // Call the global dispatcher to handle this function
        Dispatcher* dp = Dispatcher::GetInstance();
        if (dp != nullptr)
        {
            dp->Publish(new WindowResizeEvent(w, h), true);
        }
    });

    // Create Disapatcher
    Dispatcher::CreateInstance();

    // Set up IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 150";
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Implement a call to the derived class onCreate function for any implementation specific code
    bool result = onCreate();
    if (result == false)
    {
        // Cleaning up any of the GLFW Implementation code and end the application loop
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
    return result;
}

void Application::Run(const char* a_name, unsigned int a_width, unsigned int a_height, bool a_fullscreen)
{
    if (Create(a_name, a_width, a_height, a_fullscreen))
    {
        Utilities::resetTimer();
        m_running = true;
        do 
        {
            float deltaTime = Utilities::tickTimer();

            // Start the Imgui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            showFrameData(true);

            Update(deltaTime);
            Draw();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap front and back buffers
            glfwSwapBuffers(m_window); 
            // Poll for and process events
            glfwPollEvents();
        } while (m_running == true && glfwWindowShouldClose(m_window) == 0);
        Destroy();
    }
   
    // Clean up IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean Up
    ShaderUtil::DestroyInstance();
    glfwDestroyWindow(m_window);
    glfwTerminate();
    Dispatcher::DestroyInstance();
}

void Application::showFrameData(bool a_bShowFrameData)
{
    const float DISTANCE = 10.f;
    static int corner = 2;
    ImGuiIO& io = ImGui::GetIO();
    // Setting the corner that ImGui appears using the corner int var
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x + DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivat = ImVec2((corner & 1) ? 1.f : 0.f, (corner & 2) ? 1.f : 0.f); // Find where the window pos and dispkay there

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivat);
    ImGui::SetNextWindowBgAlpha(0.3f);

    if (ImGui::Begin("Frame Data", &a_bShowFrameData, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Separator();
        ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
        if (ImGui::IsMousePosValid())
        {
            ImGui::Text("MousePosition: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
        }
        else
        {
            ImGui::Text("Mouse Position: <Invalid>");
        }
    }
    ImGui::End();
}

