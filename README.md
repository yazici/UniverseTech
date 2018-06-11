# UniverseTech
A Vulkan-API engine for testing procedural content generation tools.

Dependencies:

Vulkan SDK: https://www.lunarg.com/vulkan-sdk/
STB Image: https://github.com/nothings/stb
TinyObjLoader: https://github.com/syoyo/tinyobjloader
GLM: https://glm.g-truc.net/0.9.9/index.html
GLFW: http://www.glfw.org/

Currently built as a Visual Studio 2017 Win 10 x64 project. You will need a modern graphics card with Vulkan support.

Base engine capabilities in current version:

- Obj model loading.
- 1 Texture per model.
- Display 1 model at origin.
- Frame-rate bound camera movement.
- 1 vertex and 1 fragment shader.
