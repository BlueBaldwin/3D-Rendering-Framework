#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Utilities.h"
#include <RenderFramework.h>

static double s_prevTime = 0;
static float s_totalTime = 0;
static float s_deltaTime = 0;

void Utilities::resetTimer()
{
	s_prevTime = glfwGetTime();
	s_totalTime = 0;
	s_deltaTime = 0;
}

float Utilities::tickTimer()
{
	double currentTime = glfwGetTime();
	s_deltaTime = (float)(currentTime - s_prevTime);
	s_prevTime = currentTime;
	return s_deltaTime;
}

float Utilities::getDeltaTime()
{
	return s_deltaTime;
}

float Utilities::getTotalTime()
{
	return s_totalTime;
}


char* Utilities::fileToBuffer(const char* a_sPath)
{
	// Get an fstream to read the file data
	std::fstream file;
	file.open(a_sPath, std::ios_base::in | std::ios_base::binary);
	// Test to see if the file has been opened correctly
	if (file.is_open())
	{
		// Success if file has been opened, verify the contents of the file -- i.e. check if that file is not zero length
		file.ignore(std::numeric_limits<std::streamsize>::max());		// Attempt to read the highest number of bytes from the file
		std::streamsize fileSize = file.gcount();						// gCount will have reached EOF marker, letting us know number of bytes
		file.clear();													// Clear EOF marker from being read
		file.seekg(0, std::ios_base::beg);								// seel back to the beginning of the file
		// Catch in case the file size is 0 bytes
		if (fileSize == 0)
		{
			file.close();
			std::cout << "Failed to open file because the file size is 0 bytes...";
			return nullptr;
		}
		// Create a char buffer large enough to hold the entire file
		char* dataBuffer = new char[fileSize + 1];
		memset(dataBuffer, 0, fileSize + 1);	// Ensure the contents of the buffer are cleared
		// File the buffer with the contents of the file
		file.read(dataBuffer, fileSize);
		// Close the file
		file.close();
		return dataBuffer;
	}
	return nullptr;
}

// Utility for mouse / keyboard movement of a matrix transfrom (i.e camera)
// Getting the current window for input handling and then splits the input argument matrix into it's vectors components.
// We will directly manipulate these individual componments within the function
void Utilities::freeMovement(glm::mat4& a_transform, float a_deltaTime, float a_speed, const glm::vec3& a_up)
{
    // Get the current window context
    GLFWwindow* window = glfwGetCurrentContext();
    // Get the cameras forward, right, up and location vectors
    glm::vec4 vForward = a_transform[2];
    glm::vec4 vRight = a_transform[0];
    glm::vec4 vUp = a_transform[1];
    glm::vec4 vTranslation = a_transform[3];
    // Test to see if the left shift key is pressed
    // We will use left shift to double the speed of the camera movement
    float frameSpeed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? a_deltaTime * a_speed * 2 : a_deltaTime * a_speed;

    // Checking for KEY PRESSES on the often used WASD KEYS and the QE keys to adjust the translation (Position) 
    // element of the matrix that has been passed in.
    //Translate camera 
    if (glfwGetKey(window, 'W') == GLFW_PRESS)
    {
        vTranslation -= vForward * frameSpeed;
    }
    if (glfwGetKey(window, 'S') == GLFW_PRESS)
    {
        vTranslation += vForward * frameSpeed;
    }
    if (glfwGetKey(window, 'D') == GLFW_PRESS)
    {
        vTranslation += vRight * frameSpeed;
    }
    if (glfwGetKey(window, 'A') == GLFW_PRESS)
    {
        vTranslation -= vRight * frameSpeed;
    }
    if (glfwGetKey(window, 'Q') == GLFW_PRESS)
    {
        vTranslation += vUp * frameSpeed;
    }
    if (glfwGetKey(window, 'E') == GLFW_PRESS)
    {
        vTranslation -= vUp * frameSpeed;
    }
    if (glfwGetKey(window, 'O') == GLFW_PRESS)
    {
        vTranslation = glm::vec4(10,10,10,1);
        a_transform = glm::inverse(
            glm::lookAt(
                glm::vec3(10, 10, 10), // Origin point for the camera
                glm::vec3(0, 0, 0),    // The point we want to look (origin = 0,0,0)
                glm::vec3(0, 1, 0))    // World UP Vector
        );
    }

    // Set the translation to the camera matrix that has been pased in.
    a_transform[3] = vTranslation;

    // Check for camera rotation
    // Test for mouse button being held/pressed for rotation (button 2)
    static bool sbMouseButtonDown = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
    {
        static double siPrevMouseX = 0;
        static double siPrevMouseY = 0;

        if (sbMouseButtonDown == false)
        {
            sbMouseButtonDown = true;
            glfwGetCursorPos(window, &siPrevMouseX, &siPrevMouseY);
        }

        double mouseX = 0, mouseY = 0;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        double iDeltaX = mouseX - siPrevMouseX;
        double iDeltaY = mouseY - siPrevMouseY;

        siPrevMouseX = mouseX;
        siPrevMouseY = mouseY;

        glm::mat4 mMat;

        // AXIS ANGLE CONSTRUCTION - From changes in x and y positions of the mouse
        // this delta is then used to create the current angle for the input matrix
        
        // PITCH
        if (iDeltaY != 0)
        {
            mMat = glm::axisAngleMatrix(vRight.xyz(), (float) - iDeltaY / 150.0f);
            vRight = mMat * vRight;
            vUp = mMat * vUp;
            vForward = mMat * vForward;
        }
        // YAW
        if (iDeltaX != 0)
        {
            mMat = glm::axisAngleMatrix(a_up, (float)-iDeltaX / 150.f);
            vRight = mMat * vRight;
            vUp = mMat * vUp;
            vForward = mMat * vForward;
        }

        a_transform[0] = vRight;
        a_transform[1] = vUp;
        a_transform[2] = vForward;
    }
    else
    {
        sbMouseButtonDown = false;
    }
}