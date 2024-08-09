/*
* TODO:
*/
#include "o2d.h"
#define STB_IMAGE_IMPLEMENTATION
#include "demo/stb_image.h"

uint32_t LoadTexture(const char* path) {
    int32_t width, height, channelNum;
    uint8_t *textureData = stbi_load(path, &width, &height, &channelNum, 4);
    uint32_t texture = O2D_CreateTexture(textureData, width, height);
    stbi_image_free(textureData);
    return texture;
}

int main() {
    stbi_set_flip_vertically_on_load(true);  
    O2D_Renderer renderer;
    O2D_Create(&renderer, "O2D Demo", 800, 600);
    glClearColor(0.3f, 0.5f, 0.7f, 1.0f);

    uint32_t tex1 = LoadTexture("../demo/res/knife.png");
    uint32_t tex2 = LoadTexture("../demo/res/rifle.png");

    float x = 0, y = 0;
    float w = 100;
    O2D_Quad player;
    O2D_Quad quad;
    float deltaTime = 0;
    while (O2D_WindowIsOpen(&renderer)) {
        float startTime = glfwGetTime();

        if (glfwGetKey(renderer.window, GLFW_KEY_W))
            y -= 0.1f * deltaTime;
        if (glfwGetKey(renderer.window, GLFW_KEY_S))
            y += 0.1f * deltaTime;
        if (glfwGetKey(renderer.window, GLFW_KEY_A))
            x -= 0.1f * deltaTime;
        if (glfwGetKey(renderer.window, GLFW_KEY_D))
            x += 0.1f * deltaTime;
        O2D_MakeRect(player, x, y, w, w, 0);
        O2D_MakeRect(quad, 100, 100, 200, 200, 0);

        O2D_Begin(&renderer);
        O2D_PushQuad(&renderer, quad, tex1);
        O2D_PushQuad(&renderer, player, tex1);
        O2D_End(&renderer);

        float endTime = glfwGetTime();
        deltaTime = (endTime - startTime) * 1000.0f;
    }

    O2D_Terminate(&renderer);
    return 0;
}
