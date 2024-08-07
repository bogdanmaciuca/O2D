#include "o2d.h"

int main() {
    O2D_Renderer renderer;
    O2D_Create(&renderer, "O2D Demo", 800, 600);

    O2D_Quad quad;
    quad[0].x = -100, quad[0].y = -100;
    quad[1].x = -100, quad[1].y = 100;
    quad[2].x = 100, quad[2].y = 100;
    quad[3].x = 100, quad[3].y = -100;

    while (O2D_WindowIsOpen(&renderer)) {
        O2D_Begin(&renderer);
        O2D_PushQuad(&renderer, quad);
        O2D_End(&renderer);
    }
    
    O2D_Terminate(&renderer);
    return 0;
}
