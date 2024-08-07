/*
* Features:
* - batch rendering
* - resizable window
* - sprite atlas
* - map certain parts of the batch as dynamic and only change them every frame
* - optional support for basic input
*/
#ifndef O2D_H
#define O2D_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/glad.h"
#include "inc/GLFW/glfw3.h"

#define O2D_ZeroMem(ptr, size) memset(ptr, 0, size)

enum {
    O2D_MIN_VTX_NUM = 64
};

typedef struct O2D_Vertex_t {
    float x, y; // Position
    float u, v; // Texture Coords
} O2D_Vertex;

typedef O2D_Vertex O2D_Quad[4];

typedef struct O2D_VertexBuffer_t {
    O2D_Vertex *vertices;
    uint32_t number; // Actual number of vertices
    uint32_t size;   // Allocated size
} O2D_VertexBuffer; 

typedef struct O2D_Renderer_t {
    GLFWwindow *window;
    O2D_VertexBuffer vtxBuf;
    uint32_t VAO;
    uint32_t VBO;
    uint32_t shader;
} O2D_Renderer;

bool O2D_Create(O2D_Renderer* renderer, const char* title, uint32_t width, uint32_t height);

void O2D_Terminate(O2D_Renderer* renderer);

void O2D_Begin(O2D_Renderer* renderer);

void O2D_End(O2D_Renderer* renderer);

bool O2D_WindowIsOpen(O2D_Renderer* renderer);

void O2D_PushQuad(O2D_Renderer* renderer, O2D_Quad quad);

void _O2D_EnsureBufferSize(O2D_Renderer* renderer, uint32_t requiredSize);

void _O2D_CreateShaders(O2D_Renderer* renderer);
#endif

