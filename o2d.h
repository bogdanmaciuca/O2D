#ifndef O2D_H
#define O2D_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "inc/glad.h"
#include "inc/GLFW/glfw3.h"

#define O2D_ZeroMem(ptr, size) memset(ptr, 0, size)

enum {
    O2D_MIN_VTX_NUM = 64,
    O2D_MAX_TEX_SLOTS = 32, // This value is hardcoded in the fragment shader
    O2D_MIN_INST_BUF_OFFSET_NUM = 32
};

typedef struct O2D_Vertex_t {
    float x, y; // Position
    float u, v; // Texture Coords
    float textureSlot;
} O2D_Vertex;

typedef O2D_Vertex O2D_Quad[4];

typedef struct O2D_VertexBuffer_t {
    O2D_Vertex *vertices;
    uint32_t number;
    uint32_t maxNumber; // The VBO capacity basically
    uint32_t capacity;
} O2D_VertexBuffer; 

typedef struct O2D_TextureSlotBuffer_t {
    int32_t slots[O2D_MAX_TEX_SLOTS]; // Replica of the sampler2D array from the shader
                                      // Used for reusing texture slots where possible
    int32_t capacity;                 // Maximum capacity on device
    uint16_t usedSlots;               // Occupied slots more precisely
} O2D_TextureSlotBuffer;

typedef struct O2D_Renderer_t {
    GLFWwindow *window;
    uint16_t width, height;
    float cameraX, cameraY;
    O2D_VertexBuffer vtxBuf;
    uint32_t VAO;
    uint32_t VBO;
    uint32_t shader;
    int32_t projectionMatrixUniformLocation;
    float viewProjMatrix[16];
    O2D_TextureSlotBuffer textureSlots;
} O2D_Renderer;

typedef struct O2D_Animation_t {
    uint32_t texture;
    uint16_t textureWidth, textureHeight;
    uint16_t frameIndex;
    uint16_t frameNum;
    float time;  // How much a full cycle of the animation should last (milliseconds)
    float timer; // Internal clock
} O2D_Animation;

// Initializes the renderer with basic window info
bool O2D_Create(O2D_Renderer* renderer, const char* title, uint32_t width, uint32_t height);

// Cleans up
void O2D_Terminate(O2D_Renderer* renderer);

// Should be called before pushing any geometry to the batch. Cleans the batch
void O2D_Begin(O2D_Renderer* renderer);

// Renders the batch and polls events. Should be called at the end of the frame
void O2D_End(O2D_Renderer* renderer);

// Renders the batch. Should be called at the end of the frame
void O2D_RenderBatch(O2D_Renderer* renderer);

// Clears the batch
void O2D_ClearBatch(O2D_Renderer* renderer);

// Returns true if the created window is open, false otherwise
bool O2D_WindowIsOpen(O2D_Renderer* renderer);

// Pushes quad to batch. Should be called between O2D_Begin() and O2D_End()
// calls as it may break up the batch and render it partially
void O2D_PushQuad(O2D_Renderer* renderer, O2D_Quad quad, uint32_t texture);

// Initializes O2D_Quad as a rectangle (supports rotation)
void O2D_MakeRect(O2D_Quad quad, float x, float y, float width, float height, float angle);

// Creates a OpenGL texture. Must have 4 channels
uint32_t O2D_CreateTexture(uint8_t *textureData, int32_t width, int32_t height);

// Creates an O2D_Animation object from a texture. Note: time is in milliseconds
void O2D_CreateAnimation(O2D_Animation *animation, uint32_t texture, uint32_t textureWidth,
                         uint32_t textureHeight, uint16_t frameNum, float time);

// Draws animation inside rectangle specified by (x, y, width, height) 
// Note: the UV coordinates of the O2D_Quad are altered, so it shouldn't be
// used for other drawing calls without being reinitialized
void O2D_PushAnimationFrame(O2D_Renderer* renderer, O2D_Animation *animation, O2D_Quad rect, float deltaTime);

// Sets animation timer and frameIndex to 0
void O2D_ResetAnimation(O2D_Animation *animation);

// Utility: Grows the vertex buffer capacity if necessary
void _O2D_EnsureVtxBufSize(O2D_Renderer* renderer, uint32_t requiredCapacity);

// Utility: Compiles the hard-coded shaders
void _O2D_CreateShaders(O2D_Renderer* renderer);

// Utility: Updates the projection matrix and the shader uniform
void _O2D_UpdateViewProjMatrix(O2D_Renderer* renderer);

// Utility: Translates mat by (dx, dy, 0). Note: dx and dy are in world space, not screen space
void _O2D_TranslateMatrix(O2D_Renderer* renderer, float mat[16], float dx, float dy);

// Utility: Rotates mat by angle (radians)
void _O2D_RotateMatrix(float mat[16], float angle);

// Utility: Rotates point another another point (pivot) by angle (radians)
void _O2D_RotatePoint(float *pointX, float *pointY, float pivotX, float pivotY, float angle);

#endif

