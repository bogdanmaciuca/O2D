/*
* TODO:
* - instanced rendering
* - resizing
* - drawing triangles and adding colors
* - camera rotation
*/
#include "o2d.h"
#define STB_IMAGE_IMPLEMENTATION
#include "demo/stb_image.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

uint32_t LoadTexture(const char* path) {
    int32_t width, height, channelNum;
    uint8_t *textureData = stbi_load(path, &width, &height, &channelNum, 4);
    uint32_t texture = O2D_CreateTexture(textureData, width, height);
    stbi_image_free(textureData);
    return texture;
}

void LoadAnimation(O2D_Animation *animation, const char* path, uint16_t frameNum, float time) {
    int32_t spritesheetWidth, spritesheetHeight, spritesheetChannelNum;
    uint8_t* spritesheetPixelData = stbi_load(path, &spritesheetWidth, &spritesheetHeight, &spritesheetChannelNum, 4);
    uint32_t spritesheet = O2D_CreateTexture(spritesheetPixelData, spritesheetWidth, spritesheetHeight);
    O2D_CreateAnimation(animation, spritesheet, spritesheetWidth, spritesheetHeight, frameNum, time);
    stbi_image_free(spritesheetPixelData);
}

int main() {
    stbi_set_flip_vertically_on_load(true);  
    O2D_Renderer renderer;
    O2D_Create(&renderer, "O2D Demo", 1024, 768);
    glClearColor(0.3f, 0.5f, 0.7f, 1.0f);

    O2D_Animation idle, move, shoot, reload;
    LoadAnimation(&idle, "../demo/res/idle.png", 20, 1000);
    LoadAnimation(&move, "../demo/res/move.png", 20, 800);
    LoadAnimation(&shoot, "../demo/res/shoot.png", 3, 200);
    LoadAnimation(&reload, "../demo/res/reload.png", 20, 1000);
    uint8_t state = 0;
   
    float x = 0, y = 0;
    float w = 300;
    float deltaTime = 0;
    uint64_t frameCount = 0;
    float deltaTimeSum = 0;
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
        if (glfwGetKey(renderer.window, GLFW_KEY_1))
            state = 0;
        if (glfwGetKey(renderer.window, GLFW_KEY_2))
            state = 1;
        if (glfwGetKey(renderer.window, GLFW_KEY_3))
            state = 2;
        if (glfwGetKey(renderer.window, GLFW_KEY_4))
            state = 3;


        O2D_Quad rect;
        O2D_MakeRect(rect, x, y, w, w, 0);

        O2D_Begin(&renderer);
        switch(state) {
            case 0: O2D_PushAnimationFrame(&renderer, &idle, rect, deltaTime); break;
            case 1: O2D_PushAnimationFrame(&renderer, &move, rect, deltaTime); break;
            case 2: O2D_PushAnimationFrame(&renderer, &shoot, rect, deltaTime); break;
            case 3: O2D_PushAnimationFrame(&renderer, &reload, rect, deltaTime); break;
        }
        O2D_End(&renderer);

        float endTime = glfwGetTime();
        deltaTime = (endTime - startTime) * 1000.0f;
        frameCount++;
        deltaTimeSum += deltaTime;
    }
    printf("average delta time: %f\n", deltaTimeSum / frameCount);
    O2D_Terminate(&renderer);
    return 0;
}
