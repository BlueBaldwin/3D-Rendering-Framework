#pragma once

//Forward declare the GLFWindow structure
// avoid #includes where possible

struct GLFWwindow;

class Application
{
public:
	// Constructor, sets running to false
	Application() : m_window(nullptr), m_windowHeight(0), m_windowWidth(0), m_running(false) {}
	virtual ~Application() {}

	bool Create(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool a_fullscreen);
	void Run(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool a_fullscreen);
	void Quit() { m_running = false; }

protected:
	//Pure virtual functions to be implemented by child classes
	virtual bool onCreate() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void Destroy() = 0;
	// Imgui windows
	void showFrameData(bool a_bShowFrameData);
	void ResizeWindow(bool a_bResizeWindow);
	GLFWwindow* m_window;
	unsigned int m_windowWidth;
	unsigned int m_windowHeight;

	bool m_running;

};
