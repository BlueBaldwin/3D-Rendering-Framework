#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
// GLM Includes
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Utilities.h"
#include "Shader.h"
#include "ShaderUtil.h"
#include "RenderFramework.h"

int main()
{
    RenderFramework* myApp = new RenderFramework();
    myApp->Run("Scott Baldwin's - Render Framework", 960, 540, false);
    delete myApp;
    return 0;
}
