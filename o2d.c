#include "o2d.h"

const char *_O2D_vertexShader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 uProj;\n"
    "void main() {\n"
        "gl_Position = uProj * vec4(aPos, 1.0, 1.0);\n"
    "}\n";

const char *_O2D_fragmentShader =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
        "FragColor = vec4(1, 0.4, 0.6, 1);\n"
    "}\n";

bool O2D_Create(O2D_Renderer* renderer, const char* title, uint32_t width, uint32_t height) {
    O2D_ZeroMem(renderer, sizeof(O2D_Renderer));
    renderer->width = width;
    renderer->height = height;

    if (glfwInit() == 0)
        return false;
    renderer->window = glfwCreateWindow(width, height, title, 0, 0);
    glfwMakeContextCurrent(renderer->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Could not initialize glad.\n");
        return false;
    }
    
    renderer->vtxBuf.vertices = malloc(O2D_MIN_VTX_NUM * sizeof(O2D_Vertex));
    renderer->vtxBuf.size = O2D_MIN_VTX_NUM;
    
    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);
    glBindVertexArray(renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(O2D_Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(O2D_Vertex), (void*)(2*sizeof(float)));

    _O2D_CreateShaders(renderer);
    glUseProgram(renderer->shader);
    renderer->projectionMatrixUniformLocation =
        glGetUniformLocation(renderer->shader, "uProj");
    _O2D_UpdateProjectionMatrix(renderer);

    return true;
}

void O2D_Terminate(O2D_Renderer* renderer) {
    free(renderer->vtxBuf.vertices);
}

void O2D_Begin(O2D_Renderer* renderer) {
    renderer->vtxBuf.number = 0;
    glClear(GL_COLOR_BUFFER_BIT);
}

void O2D_End(O2D_Renderer* renderer) {
    glBufferData(GL_ARRAY_BUFFER, renderer->vtxBuf.number * sizeof(O2D_Vertex),
                 renderer->vtxBuf.vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(renderer->window);
    glfwPollEvents();
}

bool O2D_WindowIsOpen(O2D_Renderer* renderer) {
    return !glfwWindowShouldClose(renderer->window);
}

void O2D_PushQuad(O2D_Renderer* renderer, O2D_Quad quad) {
    _O2D_EnsureBufferSize(renderer, renderer->vtxBuf.number + 6);
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[0];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[1];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[2];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[0];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[2];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[3];
}

void _O2D_EnsureBufferSize(O2D_Renderer* renderer, uint32_t requiredSize) {
    if (renderer->vtxBuf.size < requiredSize) {
        renderer->vtxBuf.size = requiredSize * 2;
        renderer->vtxBuf.vertices =
            realloc(renderer->vtxBuf.vertices, renderer->vtxBuf.size * sizeof(O2D_Vertex));
    }
}

void _O2D_CreateShaders(O2D_Renderer* renderer) {
    int success;
    char errorLog[512];

    // Vertex shader
    uint32_t vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &_O2D_vertexShader, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        printf("VERTEX SHADER: COMPILATION_FAILED:\n%s\n", errorLog);
    }

    // Fragment shader
    uint32_t fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &_O2D_fragmentShader,  NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        printf("FRAGMENT SHADER: COMPILATION FAILED:\n%s\n", errorLog);
    }
    
    // Linking to the final shader
    renderer->shader = glCreateProgram();
    glAttachShader(renderer->shader, vertexShader);
    glAttachShader(renderer->shader, fragmentShader);
    glLinkProgram(renderer->shader);
    glGetShaderiv(renderer->shader, GL_LINK_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(renderer->shader, 512, NULL, errorLog);
        printf("SHADER: LINKING FAILED:\n%s\n", errorLog);
    }
}

void _O2D_UpdateProjectionMatrix(O2D_Renderer* renderer) {
    float left   = renderer->cameraX - renderer->width / 2.0f;
    float right  = renderer->cameraX + renderer->width / 2.0f;
    float top    = renderer->cameraY - renderer->height / 2.0f;
    float bottom = renderer->cameraY + renderer->height / 2.0f;
    float near   = -1.0f;
    float far    = 1.0f;
    renderer->projectionMatrix[0]  = 2.0f / renderer->width;
    renderer->projectionMatrix[1]  = 0;
    renderer->projectionMatrix[2]  = 0;
    renderer->projectionMatrix[3]  = (right + left) / renderer->width;
    renderer->projectionMatrix[4]  = 0;
    renderer->projectionMatrix[5]  = -2.0f / renderer->height;
    renderer->projectionMatrix[6]  = 0;
    renderer->projectionMatrix[7]  = (top + bottom) / renderer->height;
    renderer->projectionMatrix[8]  = 0;
    renderer->projectionMatrix[9]  = 0;
    renderer->projectionMatrix[10] = -2.0f / (far - near);
    renderer->projectionMatrix[11] = -(far + near) / (far - near);
    renderer->projectionMatrix[12] = 0;
    renderer->projectionMatrix[13] = 0;
    renderer->projectionMatrix[14] = 0;
    renderer->projectionMatrix[15] = 1;

    glUniformMatrix4fv(
        renderer->projectionMatrixUniformLocation,
        1, GL_FALSE, &renderer->projectionMatrix[0]
    );
}
