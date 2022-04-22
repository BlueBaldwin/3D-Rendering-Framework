#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <imgui.h>

#include "RenderFramework.h"
#include "Dispatcher.h"
#include "ShaderUtil.h"
#include "Utilities.h"
#include "TextureManager.h"
#include "obj_loader.h"
#include "Texture.h"
#include "ApplicationEvent.h"
#include "Texture.h"


RenderFramework::RenderFramework()
{
}

RenderFramework::~RenderFramework()
{
}

#pragma region OnCreate
// Setting up our application - So the Update loop doesn't have to fully re-implement the 3D world each frame
bool RenderFramework::onCreate()
{
    // Getting an instance of our dispatcher
    Dispatcher* dp = Dispatcher::GetInstance();
    if (dp)
    {
        // Subscribing our window resize member function
        dp->Subscribe(this, &RenderFramework::onWindowResize);
    }

    // Get an instance of the texture manager
    TextureManager::CreateInstance();

    // Set the clear colour and enable depth testing and backface culling
    m_backgroundColour = glm::vec3(0.67f, 0.25f, 0.05f);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //create shader program
    unsigned int vertexShader = ShaderUtil::loadShader("resource/shaders/vertex.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = ShaderUtil::loadShader("resource/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    m_uiProgram = ShaderUtil::createProgram(vertexShader, fragmentShader);

    // Create a grid of lines to be drawn during our update
    // Create a 10x10 square grid
    m_lines = new Line[42];

    for (int i = 0, j = 0; i < 21; ++i, j += 2)
    {
        m_lines[j].v0.position = glm::vec4(-10 + i, 0.f, 10.f, 1.f);
        m_lines[j].v0.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
        m_lines[j].v1.position = glm::vec4(-10 + i, 0.f, -10.f, 1.f);
        m_lines[j].v1.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);

        m_lines[j + 1].v0.position = glm::vec4(10, 0.f, -10.f + i, 1.f);
        m_lines[j + 1].v0.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
        m_lines[j + 1].v1.position = glm::vec4(-10, 0.f, -10.f + i, 1.f);
        m_lines[j + 1].v1.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
    }

    // Create a vertex buffer to hold our line data

    glGenBuffers(1, &m_lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    // Fill vertex buffer with line data
    glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), m_lines, GL_STATIC_DRAW);

    // enables the vertex array state, since we're sending in an array of vertices
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Specify where our vertex array is, how many components each vertex has,
    // the data type of each component and whether the data is normalised or not
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create a world-space matrix for a camera
    m_cameraMatrix =
        glm::inverse(
            glm::lookAt(
                glm::vec3(10, 10, 10), // Origin point for the camera
                glm::vec3(0, 0, 0),    // The point we want to look (origin = 0,0,0)
                glm::vec3(0, 1, 0))    // World UP Vector
        );

    // Create a prospective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
    // Using the look at function above we create an object (Matrix)
    m_projectionMatrix =
        glm::perspective(
            glm::pi<float>() * 0.25f,
            m_windowWidth / (float)m_windowHeight,
            0.1f, 1000.0f);

#pragma region Model & Material Loading

    m_specularTint = glm::vec3(1.f, 0.f, 0.f);
    m_objModel = new OBJModel();
    if (m_objModel->load("resource/models/D0208009.obj"))
    {
        TextureManager* pTM = TextureManager::GetInstance();
        // Load in texture for model if any are present
        for (int i = 0; i < m_objModel->getMaterialCount(); ++i)
        {
            OBJMaterial* mat = m_objModel->getMaterialByIndex(i);
            for (int n = 0; n < OBJMaterial::TextureTypes::TextureTypes_Count; ++n)
            {
                if (mat->textureFileNames[n].size() > 0)
                {
                    unsigned int textureID = pTM->LoadTexture(mat->textureFileNames[n].c_str());
                    mat->textureIDs[n] = textureID;
                }
            }
        }
        // Setup shaders for obj model rendering
        // create OBJ shader prograp[']

        unsigned int obj_vertexShader = ShaderUtil::loadShader("resource/shaders/obj_vertex.glsl", GL_VERTEX_SHADER);
        unsigned int obj_fragmentShader = ShaderUtil::loadShader("resource/shaders/obj_fragment.glsl", GL_FRAGMENT_SHADER);
        m_objProgram = ShaderUtil::createProgram(obj_vertexShader, obj_fragmentShader);
        // Set up vertex and index buffer for OBJ rendering
        glGenBuffers(2, m_objModelBuffer);
        // Set up vertex buffer data
        glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else {
        std::cout << "Failed to Load Model" << std::endl;
        return false;
    }
#pragma endregion Model & Material Loading

#pragma region Skybox
    // Skybox

    std::vector<std::string> textures_faces = { "resource/skybox/right.jpg", "resource/skybox/left.jpg",
                                                 "resource/skybox/top.jpg", "resource/skybox/bottom.jpg",
                                                 "resource/skybox/front.jpg", "resource/skybox/back.jpg" };
    unsigned int cubemap_image_tag[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, };

    Texture* pTexture = new Texture();
    m_CubeMaptextID = pTexture->LoadCubeMap(textures_faces, cubemap_image_tag);
    
    vertexShader = ShaderUtil::loadShader("resource/shaders/skybox_vertex.glsl",GL_VERTEX_SHADER);
    fragmentShader = ShaderUtil::loadShader("resource/shaders/skybox_fragment.glsl",GL_FRAGMENT_SHADER);
    m_SBProgramID = ShaderUtil::createProgram(vertexShader, fragmentShader);

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // Create a vertex buffer for the skybox
    glGenBuffers(1, &m_SBVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SBVBO);
    // Fill vertex buffer with line data
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(float) * 108, skyboxVertices, 0);
    // Generate our vertex array object
    glGenVertexArrays(1, &m_SBVAO);
    glBindVertexArray(m_SBVAO);

    // enable the vertex array state, since we're sending in an array of vertices
    glEnableVertexAttribArray(0);

    // Specify where our vertex array is, how many components each vertex has, the data type for each component, and whether the data is normalised
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_SBVBO);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

#pragma endregion Skybox

#pragma endregion OnCreate

void RenderFramework::Update(float deltaTime)
{
    // Updating the camera matrix based on mouse and keyboard input

    Utilities::freeMovement(m_cameraMatrix, deltaTime, 3.f);
    // Implementing IMGUI windows
    
    MainMenu(m_bMy_tool_active);
    if (m_changeColour)
    {
        ChangeBackgroundColour(&m_backgroundColour, &m_specularTint);
    }
  
}

void RenderFramework::Draw()
{ 
    glDepthFunc(GL_LESS);
    // Clear the backbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    glClearColor(m_backgroundColour.x, m_backgroundColour.y, m_backgroundColour.z, 1.f);

    // Get the view matrix from the world-space camera matrix
    glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
    glm::mat4 projectionViewMatrix = m_projectionMatrix * viewMatrix;

    //Enable shaders
    glUseProgram(m_uiProgram);

    // Send the projection matrix to the vertex shader
    // Ask the shader program for the location of the projection-view matrix uniform variable
    unsigned int projectionViewUniformLocation = glGetUniformLocation(m_uiProgram, "ProjectionViewMatrix");
    // Send this location a pointer to our glm::mat4 (send across float data)
    glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));

    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), m_lines, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Specify where our vertex array is, how many components each vertex has,
    // the data type of each component and whehter the data is normalised or not
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

    glDrawArrays(GL_LINES, 0, 42 * 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);

    glUseProgram(m_objProgram);
    // Set the projection view matrix for this shader
    projectionViewUniformLocation = glGetUniformLocation(m_objProgram, "ProjectionViewMatrix");
    glUniformMatrix4fv(projectionViewUniformLocation, 1, GL_FALSE, glm::value_ptr(projectionViewMatrix));

    for (int i = 0; i < m_objModel->getMeshCount(); ++i)
    {
        // Get the model matrix location from the shader program
        int modelMatrirxUniformLocation = glGetUniformLocation(m_objProgram, "ModelMatrix");
        // Send the OBJ model's world matrix data across to the shader program
        glUniformMatrix4fv(modelMatrirxUniformLocation, 1, false, glm::value_ptr(m_objModel->getWorldMatrix()));

        int cameraPositionUniformLocation = glGetUniformLocation(m_objProgram, "camPos");
        glUniform4fv(cameraPositionUniformLocation, 1, glm::value_ptr(m_cameraMatrix[3]));
        
        OBJMesh* pMesh = m_objModel->getMeshByIndex(i);
        // Send material data to shader
        int kA_location = glGetUniformLocation(m_objProgram, "kA");
        int kD_location = glGetUniformLocation(m_objProgram, "kD");
        int kS_location = glGetUniformLocation(m_objProgram, "kS");

        OBJMaterial* pMaterial = pMesh->m_material;
        if (pMaterial != nullptr)
        {
            // Send the OBJ Model's world matrix data across to the shader program
            glUniform4fv(kA_location, 1, glm::value_ptr(pMaterial->Get_kA()));
            glUniform4fv(kD_location, 1, glm::value_ptr(pMaterial->Get_kD()));
            glUniform4fv(kS_location, 1, glm::value_ptr(pMaterial->Get_kS()));

            // Get the location of the diffuse texture
            int texUniformLoc = glGetUniformLocation(m_objProgram, "DiffuseTexture");
            glUniform1i(texUniformLoc, 0); // set diffuse texture to be GL_Texture0

            glActiveTexture(GL_TEXTURE0); // set the active texture unit to texture0
            // Bind the texture for diffuse for this material to the texture 0
            glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::DiffuseTexture]);

            // Get the location of the diffuse texture
            texUniformLoc = glGetUniformLocation(m_objProgram, "SpecularTexture");
            glUniform1i(texUniformLoc, 1); // set diffuse texture to be GL_Texturel

            glActiveTexture(GL_TEXTURE1); // set the active texture unit to texture1

            int specularTintUniform = glGetUniformLocation(m_objProgram, "specularTint");
            glUniform3fv(specularTintUniform, 1, glm::value_ptr(m_specularTint));

            // Bind the texture for diffuse for this material to the texture0
            glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::SpecularTexture]);

            // Get the location of the diffuse texture
            texUniformLoc = glGetUniformLocation(m_objProgram, "NormalTexture");
            glUniform1i(texUniformLoc, 2); // Set diffuse texture to be GL_Texture2

            glActiveTexture(GL_TEXTURE2); // Set the active texture unit to texture2
            // Bind the texture for diffuse for this material to the texture
            glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::NormalTexture]);

        }
        else // No material to obtain lighting information from use defaults
        {
            glUniform4fv(kA_location, 1, glm::value_ptr(glm::vec4(0.25f, 0.25f, 0.25f, 1.f)));
            glUniform4fv(kD_location, 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 1.f)));
            glUniform4fv(kS_location, 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 64.f)));
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(OBJVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_objModelBuffer[1]);
        glEnableVertexAttribArray(0);   // Position
        glEnableVertexAttribArray(1);   // Normal 
        glEnableVertexAttribArray(2);   // UV Coord

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::PositionOffset);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::NormalOffset);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::UVCoordOffset);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    //glUseProgram(0);

    // Draw the Skybox
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_SBProgramID);
    //glDepthMask(GL_FALSE);

    //projectionViewMatrix = glm::mat4(glm::mat3(projectionViewMatrix));
    projectionViewUniformLocation = glGetUniformLocation(m_SBProgramID, "ProjectionViewMatrix");
    glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));

    glBindVertexArray(m_SBVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMaptextID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    glUseProgram(0);


}

void RenderFramework::Destroy()
{
    delete m_objModel;
    delete[] lines;
    glDeleteBuffers(1, &m_lineVBO);
    ShaderUtil::deleteProgram(m_uiProgram);
    TextureManager::DestroyInstance();
    ShaderUtil::DestroyInstance();
}

void RenderFramework::onWindowResize(WindowResizeEvent* e)
{
    std::cout << "Member event handler called" << std::endl;
    if (e->GetWidth() > 0 && e->GetHeight() > 0)
    {
        m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, e->GetWidth() / (float)e->GetHeight(), 0.1f, 1000.0f);
        glViewport(0, 0, e->GetWidth(), e->GetHeight());
        e->Handled();
    }
}

void RenderFramework::ChangeBackgroundColour(glm::vec3* a_backgroundColor, glm::vec3* a_specilarTint)
{
    // Set up an ImGui window to control background colour
    const float X_DISTANCE = 10.f;
    const float Y_DISTANCE = 40.f;
    static int corner = 0;
    
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_size = ImVec2(400.f, 100.f);
   // ImVec2 window_pos = ImVec2(io.DisplaySize.x * 0.01f, io.DisplaySize.y * 0.9f);
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - X_DISTANCE : X_DISTANCE, (corner & 2) ? io.DisplaySize.y - Y_DISTANCE : Y_DISTANCE);
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    if (ImGui::Begin("Set Background Colour", &m_changeColour))
    {
        ImGui::ColorEdit3("Background Colour: ", glm::value_ptr(*a_backgroundColor));
        ImGui::ColorEdit3("Specular Tint", glm::value_ptr(*a_specilarTint));
    }
    ImGui::End();   // Regardless as to weather or not this ImGui::Begin was called or not, then it needs to end.
}

void RenderFramework::MainMenu(bool& m_bMy_tool_active)
{
   
    if (m_bMy_tool_active)
    {
     // Create a window menu bar.
        ImVec2 window_size = ImVec2(1920.f, 25.f);
        ImVec2 window_pos = ImVec2(0.f, 0.f);
        ImGui::SetNextWindowPos(window_pos);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        ImGui::Begin("Main Menu", &m_bMy_tool_active, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
                if (ImGui::MenuItem("Colour Panel")) { m_changeColour = true; }
                if (ImGui::MenuItem("Close", "Ctrl+W")) { Application::Quit(); }    
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Edit a color (stored as ~4 floats)
       // ImGui::ColorEdit4("Color", my_color);

        // Plot some values
        const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
        ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

        // Display contents in a scrolling region
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
        ImGui::BeginChild("Scrolling");
        for (int n = 0; n < 50; n++)
            ImGui::Text("%04d: Some text", n);
        ImGui::EndChild();
        ImGui::End();
    }
   
}
