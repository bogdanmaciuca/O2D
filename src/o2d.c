#include "../include/o2d.h"

const char *_O2D_vertexShader =
    "#version 450 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "layout (location = 2) in float aTexSlot;\n"
    "out vec2 oTexCoord;\n"
    "out float oTexSlot;\n"
    "uniform mat4 uViewProj;\n"
    "void main() {\n"
        "oTexCoord = aTexCoord;\n"
        "oTexSlot = aTexSlot;\n"
        "gl_Position = uViewProj * vec4(aPos, 1.0, 1.0);\n"
    "}\n";

const char *_O2D_fragmentShader =
    "#version 450 core\n"
    "out vec4 FragColor;\n"
    "in vec2 oTexCoord;\n"
    "in float oTexSlot;\n"
    "uniform sampler2D uTextures[32];\n" // Supports a maximum of 32 texture slots
    "void main() {\n"
        "FragColor = texture(uTextures[uint(oTexSlot)], oTexCoord);\n"
    "}\n";

void _O2D_WindowResizeCallback(GLFWwindow *window, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
}
bool O2D_Create(O2D_Renderer* renderer, const char* title, uint32_t width, uint32_t height) {
    O2D_ZeroMem(renderer, sizeof(O2D_Renderer));
    renderer->width = width;
    renderer->height = height;
    if (glfwInit() == 0)
        return false;
    renderer->window = glfwCreateWindow(width, height, title, 0, 0);
    glfwSetWindowAspectRatio(renderer->window, renderer->width, renderer->height);
    glfwSetFramebufferSizeCallback(renderer->window, _O2D_WindowResizeCallback);

    glfwMakeContextCurrent(renderer->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Could not initialize glad.\n");
        return false;
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer->vtxBuf.vertices = malloc(O2D_MIN_VTX_NUM * sizeof(O2D_Vertex));
    renderer->vtxBuf.capacity = O2D_MIN_VTX_NUM;

    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);
    glBindVertexArray(renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(O2D_Vertex), (void*)offsetof(O2D_Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(O2D_Vertex), (void*)offsetof(O2D_Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(O2D_Vertex), (void*)offsetof(O2D_Vertex, textureSlot));

    _O2D_CreateShaders(renderer);
    glUseProgram(renderer->shader);
    renderer->projectionMatrixUniformLocation =
        glGetUniformLocation(renderer->shader, "uViewProj");
    _O2D_UpdateViewProjMatrix(renderer);
    // Fill the texture slots
    uint32_t location = glGetUniformLocation(renderer->shader, "uTextures");
    const int samplers[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };
    glUniform1iv(location, O2D_MAX_TEX_SLOTS, &samplers[0]);

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &renderer->textureSlots.capacity);

    return true;
}

void O2D_Terminate(O2D_Renderer* renderer) {
    free(renderer->vtxBuf.vertices);
}

void O2D_Begin(O2D_Renderer* renderer) {
    O2D_ClearBatch(renderer);
    glClear(GL_COLOR_BUFFER_BIT);
}

void O2D_End(O2D_Renderer* renderer) {
    O2D_RenderBatch(renderer);
    glfwSwapBuffers(renderer->window);
    glfwPollEvents();
}

void O2D_RenderBatch(O2D_Renderer* renderer) {
    glUseProgram(renderer->shader);
    _O2D_UpdateViewProjMatrix(renderer);
    // Only reallocate data if the number of vertices exceeds the vertex buffer capacity
    if (renderer->vtxBuf.number > renderer->vtxBuf.maxNumber) {
        renderer->vtxBuf.maxNumber = renderer->vtxBuf.number;
        glBufferData(GL_ARRAY_BUFFER, renderer->vtxBuf.number * sizeof(O2D_Vertex),
                     renderer->vtxBuf.vertices, GL_DYNAMIC_DRAW);
    }
    else {
        glBufferSubData(
            GL_ARRAY_BUFFER, 0, renderer->vtxBuf.number * sizeof(O2D_Vertex),
            renderer->vtxBuf.vertices
        );
    }
    // Render
    glBindVertexArray(renderer->VAO);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vtxBuf.number);
}

void O2D_ClearBatch(O2D_Renderer *renderer) {
    renderer->vtxBuf.number = 0;
    renderer->textureSlots.usedSlots = 0;
    for (int i = 0; i < O2D_MAX_TEX_SLOTS; i++)
renderer->textureSlots.slots[i] = O2D_MAX_TEX_SLOTS;
}

bool O2D_WindowIsOpen(O2D_Renderer* renderer) {
    return !glfwWindowShouldClose(renderer->window);
}

void O2D_PushQuad(O2D_Renderer* renderer, O2D_Quad quad, uint32_t texture) {
    // Check if texture already exists in the current batch
    int16_t texSlot = -1;
    for (int16_t i = 0; i < renderer->textureSlots.capacity; i++) {
        if (renderer->textureSlots.slots[i] == texture) {
            texSlot = i;
            break;
        }
    }
    // If the texture doesn't exists, push it to the next available slot
    if (texSlot == -1) {
        // If the texture slots are full, render the batch as it is
        if (renderer->textureSlots.usedSlots >= O2D_MAX_TEX_SLOTS) {
            O2D_RenderBatch(renderer);
            O2D_ClearBatch(renderer);
        }
        texSlot = renderer->textureSlots.usedSlots++;
        renderer->textureSlots.slots[texSlot] = texture;
        glBindTextureUnit(texSlot, texture);
    }

    for (uint8_t i = 0; i < 4; i++)
        quad[i].textureSlot = texSlot;
     _O2D_EnsureVtxBufSize(renderer, renderer->vtxBuf.number + 6);
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[0];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[1];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[2];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[0];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[2];
    renderer->vtxBuf.vertices[renderer->vtxBuf.number++] = quad[3];
}

void O2D_MakeRect(O2D_Quad quad, float x, float y, float width, float height, float angle) {
    quad[0] = (O2D_Vertex){ x - width / 2.0f, y - height / 2.0f, 0.0f, 1.0f };
    quad[1] = (O2D_Vertex){ x - width / 2.0f, y + height / 2.0f, 0.0f, 0.0f };
    quad[2] = (O2D_Vertex){ x + width / 2.0f, y + height / 2.0f, 1.0f, 0.0f };
    quad[3] = (O2D_Vertex){ x + width / 2.0f, y - height / 2.0f, 1.0f, 1.0f };
    if (angle != 0.0f) {
        for (uint8_t i = 0; i < 4; i++)
            _O2D_RotatePoint(&quad[i].x, &quad[i].y, x, y, angle);
    }
}

uint32_t O2D_CreateTexture(uint8_t *textureData, int32_t width, int32_t height) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void O2D_CreateAnimation(O2D_Animation *animation, uint32_t texture, uint32_t textureWidth, uint32_t textureHeight, uint16_t frameNum, float time) {
    animation->texture = texture;
    animation->textureWidth = textureWidth;
    animation->textureHeight = textureHeight;
    animation->frameNum = frameNum;
    animation->time = time;
    animation->timer = 0.0f;
}

void O2D_PushAnimationFrame(O2D_Renderer *renderer, O2D_Animation *animation, O2D_Quad rect, float deltaTime) {
    if (animation->timer > animation->time / animation->frameNum) {
        animation->timer -= animation->time / animation->frameNum;
        animation->frameIndex++;
    }
    if (animation->frameIndex == animation->frameNum)
        animation->frameIndex = 0;
    rect[0].u = rect[1].u = (float)animation->frameIndex / animation->frameNum;
    rect[2].u = rect[3].u = (float)(animation->frameIndex + 1) / animation->frameNum;
    O2D_PushQuad(renderer, rect, animation->texture);
    animation->timer += deltaTime;
}

void O2D_ResetAnimation(O2D_Animation *animation) {
    animation->frameIndex = 0;
    animation->timer = 0;
}

void _O2D_EnsureVtxBufSize(O2D_Renderer *renderer, uint32_t requiredCapacity) {
    if (renderer->vtxBuf.capacity < requiredCapacity) {
        renderer->vtxBuf.capacity = requiredCapacity * 2;
        renderer->vtxBuf.vertices =
            realloc(renderer->vtxBuf.vertices, renderer->vtxBuf.capacity * sizeof(O2D_Vertex));
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

void _O2D_UpdateViewProjMatrix(O2D_Renderer* renderer) {
    float left   = -renderer->width / 2.0f;
    float right  = renderer->width / 2.0f;
    float top    = -renderer->height / 2.0f;
    float bottom = renderer->height / 2.0f;
    float near   = -1.0f;
    float far    = 1.0f;
    renderer->viewProjMatrix[0]  = 2.0f / renderer->width;
    renderer->viewProjMatrix[1]  = 0;
    renderer->viewProjMatrix[2]  = 0;
    renderer->viewProjMatrix[3]  = (right + left) / renderer->width;
    renderer->viewProjMatrix[4]  = 0;
    renderer->viewProjMatrix[5]  = -2.0f / renderer->height;
    renderer->viewProjMatrix[6]  = 0;
    renderer->viewProjMatrix[7]  = (top + bottom) / renderer->height;
    renderer->viewProjMatrix[8]  = 0;
    renderer->viewProjMatrix[9]  = 0;
    renderer->viewProjMatrix[10] = -2.0f / (far - near);
    renderer->viewProjMatrix[11] = -(far + near) / (far - near);
    renderer->viewProjMatrix[12] = 0;
    renderer->viewProjMatrix[13] = 0;
    renderer->viewProjMatrix[14] = 0;
    renderer->viewProjMatrix[15] = 1;

    _O2D_TranslateMatrix(renderer, renderer->viewProjMatrix, renderer->cameraX, renderer->cameraY);

    glUniformMatrix4fv(
        renderer->projectionMatrixUniformLocation,
        1, GL_FALSE, &renderer->viewProjMatrix[0]
    );
}

void _O2D_RotatePoint(float *pointX, float *pointY, float pivotX, float pivotY, float angle) {
  float s = sin(angle);
  float c = cos(angle);

  // translate point back to origin:
  *pointX -= pivotX;
  *pointY -= pivotY;

  // rotate point
  float xnew = *pointX * c - *pointY * s;
  float ynew = *pointX * s + *pointY * c;

  // translate point back:
  *pointX = xnew + pivotX;
  *pointY = ynew + pivotY;
}

void _O2D_TranslateMatrix(O2D_Renderer* renderer, float mat[16], float dx, float dy) {
    mat[12] -= dx / renderer->width * 2;
    mat[13] += dy / renderer->height * 2;
}

//template<typename T, qualifier Q>
//GLM_FUNC_QUALIFIER mat<4, 4, T, Q> rotate(mat<4, 4, T, Q> const& m, T angle, vec<3, T, Q> const& v)
//{
//    T const a = angle;
//    T const c = cos(a);
//    T const s = sin(a);
//
//    vec<3, T, Q> axis(normalize(v));
//    vec<3, T, Q> temp((T(1) - c) * axis);
//
//    mat<4, 4, T, Q> Rotate;
//    Rotate[0][0] = c + temp[0] * axis[0];
//    Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
//    Rotate[0][2] = temp[0] * axis[2] - s * axis[1];
//
//    Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
//    Rotate[1][1] = c + temp[1] * axis[1];
//    Rotate[1][2] = temp[1] * axis[2] + s * axis[0];
//
//    Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
//    Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
//    Rotate[2][2] = c + temp[2] * axis[2];
//
//    mat<4, 4, T, Q> Result;
//    Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
//    Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
//    Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
//    Result[3] = m[3];
//}

