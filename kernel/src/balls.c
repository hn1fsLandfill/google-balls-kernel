#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math.h"
#include "graphics/graphics.h"
#include "x86.h"
#include "balls.h"
#include "mem.h"

typedef struct {
    double x, y, z;
} Vector3;

typedef struct {
    unsigned char r, g, b, a; // 0..1
} Color;

typedef struct {
    Vector3 curPos, originalPos, targetPos, velocity;
    Color color;
    double radius, size;
    double friction;
    double springStrength;
} Point;

typedef struct {
    Vector3 mousePos;
    Point* points;
    size_t count;
} PointCollection;

typedef struct {
    PointCollection pc;
    char running;
    int width;
    int height;
} App;

typedef struct {
    int x, y;
    int size;
    const uint32_t color;
} PointData;

static const double PI = 3.14159265359; // why not just pi :sob: (and then just multiply it)

static const PointData pointData[] = {
    {202, 78, 9, 0xed9d33}, {348, 83, 9, 0xd44d61}, {256, 69, 9, 0x4f7af2},
    {214, 59, 9, 0xef9a1e}, {265, 36, 9, 0x4976f3}, {300, 78, 9, 0x269230},
    {294, 59, 9, 0x1f9e2c}, {45, 88, 9, 0x1c48dd}, {268, 52, 9, 0x2a56ea},
    {73, 83, 9, 0x3355d8}, {294, 6, 9, 0x36b641}, {235, 62, 9, 0x2e5def},
    {353, 42, 8, 0xd53747}, {336, 52, 8, 0xeb676f}, {208, 41, 8, 0xf9b125},
    {321, 70, 8, 0xde3646}, {8, 60, 8, 0x2a59f0}, {180, 81, 8, 0xeb9c31},
    {146, 65, 8, 0xc41731}, {145, 49, 8, 0xd82038}, {246, 34, 8, 0x5f8af8},
    {169, 69, 8, 0xefa11e}, {273, 99, 8, 0x2e55e2}, {248, 120, 8, 0x4167e4},
    {294, 41, 8, 0x0b991a}, {267, 114, 8, 0x4869e3}, {78, 67, 8, 0x3059e3},
    {294, 23, 8, 0x10a11d}, {117, 83, 8, 0xcf4055}, {137, 80, 8, 0xcd4359},
    {14, 71, 8, 0x2855ea}, {331, 80, 8, 0xca273c}, {25, 82, 8, 0x2650e1},
    {233, 46, 8, 0x4a7bf9}, {73, 13, 8, 0x3d65e7}, {327, 35, 6, 0xf47875},
    {319, 46, 6, 0xf36764}, {256, 81, 6, 0x1d4eeb}, {244, 88, 6, 0x698bf1},
    {194, 32, 6, 0xfac652}, {97, 56, 6, 0xee5257}, {105, 75, 6, 0xcf2a3f},
    {42, 4, 6, 0x5681f5}, {10, 27, 6, 0x4577f6}, {166, 55, 6, 0xf7b326},
    {266, 88, 6, 0x2b58e8}, {178, 34, 6, 0xfacb5e}, {100, 65, 6, 0xe02e3d},
    {343, 32, 6, 0xf16d6f}, {59, 5, 6, 0x507bf2}, {27, 9, 6, 0x5683f7},
    {233, 116, 6, 0x3158e2}, {123, 32, 6, 0xf0696c}, {6, 38, 6, 0x3769f6},
    {63, 62, 6, 0x6084ef}, {6, 49, 6, 0x2a5cf4}, {108, 36, 6, 0xf4716e},
    {169, 43, 6, 0xf8c247}, {137, 37, 6, 0xe74653}, {318, 58, 6, 0xec4147},
    {226, 100, 5, 0x4876f1}, {101, 46, 5, 0xef5c5c}, {226, 108, 5, 0x2552ea},
    {17, 17, 5, 0x4779f7}, {232, 93, 5, 0x4b78f1}
};
static const size_t N = sizeof(pointData) / sizeof(pointData[0]);

// Logo size calculation shit (TO FUCKING MAKE IT CENTERED AND RESPONSIVE FUCK)
static void compute_pointdata_bounds(double* w, double* h) {
    int minX = 9999, maxX = -9999;
    int minY = 9999, maxY = -9999;

    for (size_t i = 0; i < N; i++) {
        if (pointData[i].x < minX) minX = pointData[i].x;
        if (pointData[i].x > maxX) maxX = pointData[i].x;
        if (pointData[i].y < minY) minY = pointData[i].y;
        if (pointData[i].y > maxY) maxY = pointData[i].y;
    }

    *w = maxX - minX;
    *h = maxY - minY;
}

// Utilities
#if 0
static Color color_from_hex(const char* hex) {
    Color c = {1.0, 1.0, 1.0, 1.0};
    if (!hex) return c;
    const char* p = hex;
    if (p[0] == '#') p++;
    unsigned int value = 0;
    if (strlen(p) == 6) {
        value = (unsigned int)strtoul(p, NULL, 16);
        c.r = ((value >> 16) & 0xFF) / 255.0;
        c.g = ((value >> 8) & 0xFF) / 255.0;
        c.b = (value & 0xFF) / 255.0;
        c.a = 1.0;
    }
    return c;
}
#endif

// Physics and rendering
static void point_update(Point* p) {
    // X axis spring physics
    double dx = p->targetPos.x - p->curPos.x;
    double ax = dx * p->springStrength;
    p->velocity.x += ax;
    p->velocity.x *= p->friction;

    if (fabs(dx) < 0.1 && fabs(p->velocity.x) < 0.01) {
        p->curPos.x = p->targetPos.x;
        p->velocity.x = 0.0;
    } else {
        p->curPos.x += p->velocity.x;
    }

    // Y axis spring physics
    double dy = p->targetPos.y - p->curPos.y;
    double ay = dy * p->springStrength;
    p->velocity.y += ay;
    p->velocity.y *= p->friction;

    if (fabs(dy) < 0.1 && fabs(p->velocity.y) < 0.01) {
        p->curPos.y = p->targetPos.y;
        p->velocity.y = 0.0;
    } else {
        p->curPos.y += p->velocity.y;
    }

    // Z axis (depth) based on distance from original position
    double dox = p->originalPos.x - p->curPos.x;
    double doy = p->originalPos.y - p->curPos.y;
    double dd = (dox * dox) + (doy * doy);
    double d = sqrt(dd);

    p->targetPos.z = d / 100.0 + 1.0;
    double dz = p->targetPos.z - p->curPos.z;
    double az = dz * p->springStrength;
    p->velocity.z += az;
    p->velocity.z *= p->friction;

    if (fabs(dz) < 0.01 && fabs(p->velocity.z) < 0.001) {
        p->curPos.z = p->targetPos.z;
        p->velocity.z = 0.0;
    } else {
        p->curPos.z += p->velocity.z;
    }

    // Update radius based on depth
    p->radius = p->size * p->curPos.z;
    if (p->radius < 1.0) p->radius = 1.0;
}

static void point_draw(Point* p) {
    dot(p->curPos.x, p->curPos.y, p->radius, (p->color.b << 0) | (p->color.g << 8) | (p->color.r << 16) | (0xff << 24));
}

static void point_collection_update(PointCollection* pc) {
    for (size_t i = 0; i < pc->count; ++i) {
        Point* point = &pc->points[i];

        double dx = pc->mousePos.x - point->curPos.x;
        double dy = pc->mousePos.y - point->curPos.y;
        double d = sqrt(dx * dx + dy * dy);

        if (d < 150.0) {
            point->targetPos.x = point->curPos.x - dx;
            point->targetPos.y = point->curPos.y - dy;
        } else {
            point->targetPos.x = point->originalPos.x;
            point->targetPos.y = point->originalPos.y;
        }

        point_update(point);
    }
}

static void point_collection_draw(PointCollection* pc) {
    for (size_t i = 0; i < pc->count; ++i) {
        point_draw(&pc->points[i]);
    }
}

unsigned char points[32768] = {0};

// App setup
static void app_init_points(App* app) {
    // sizeof(Point)*N    
    // TODO
    app->pc.points = (Point *)malloc(N*sizeof(Point));
    //app->pc.points = (void *)&points;
    app->pc.count = N;

    double logoW, logoH;
    compute_pointdata_bounds(&logoW, &logoH);

    // divide it by 2 to be in the center (vewy logic :nerd:)
    double centerX = (app->width / 2.0) - (logoW / 2.0);
    double centerY = (app->height / 2.0) - (logoH / 2.0);

    for (size_t i = 0; i < N; ++i) {
        Point* p = &app->pc.points[i];
        double x = centerX + pointData[i].x;
        double y = centerY + pointData[i].y;

        p->curPos.x = x; p->curPos.y = y; p->curPos.z = 0.0;
        p->originalPos = p->curPos;
        p->targetPos = p->curPos;
        p->velocity.x = p->velocity.y = p->velocity.z = 0.0;
        p->size = (double)pointData[i].size;
        p->radius = p->size;
        p->friction = 0.8;
        p->springStrength = 0.1;
        p->color = (Color){ .r = pointData[i].color >> 16 & 0xff, .g = pointData[i].color >> 8 & 0xff, .b = pointData[i].color & 0xff };
    }
}

// left right up down
unsigned char keys[] = {0,0,0,0};

#define SPEED 16

void update(App *app) {
    point_collection_update(&app->pc);
}

char cursor[] = {
    1,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,0,0,
    1,2,2,2,2,1,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,0,0,
    1,2,2,2,2,2,2,1,0,0,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,0,0,
    1,2,2,2,2,2,2,2,2,1,0,0,0,
    1,2,2,2,2,2,2,2,2,2,1,0,0,
    1,2,2,2,2,2,2,1,1,1,1,1,0,
    1,2,2,2,2,2,2,1,0,0,0,0,0,
    1,2,2,2,1,2,2,1,0,0,0,0,0,
    1,2,2,1,1,2,2,2,1,0,0,0,0,
    1,2,1,0,0,1,2,2,1,0,0,0,0,
    1,1,0,0,0,0,1,2,2,1,0,0,0,
    1,0,0,0,0,0,1,2,2,1,0,0,0,
    0,0,0,0,0,0,0,1,2,2,1,0,0,
    0,0,0,0,0,0,0,1,2,2,1,0,0,
    0,0,0,0,0,0,0,0,1,1,0,0,0,
};

void input(App *app) {
    unsigned char k = ps2_poll();
    while(k != 0x00) {
        if (k == 0xE0) {
            k = ps2_poll();
            switch(k) {
                case 0x4B:
                    keys[0] = 1;
                    break;
                case 0xCB:
                    keys[0] = 0;
                    break;
                case 0x4D:
                    keys[1] = 1;
                    break;
                case 0xCD:
                    keys[1] = 0;
                    break;
                case 0x48:
                    keys[2] = 1;
                    break;
                case 0xC8:
                    keys[2] = 0;
                    break;
                case 0x50:
                    keys[3] = 1;
                    break;
                case 0xD0:
                    keys[3] = 0;
                    break;
            }
        }
        k = ps2_poll();
    }

    if(keys[0]) {
        app->pc.mousePos.x -= SPEED;
    }
    if(keys[1]) {
        app->pc.mousePos.x += SPEED;
    }
    if(keys[2]) {
        app->pc.mousePos.y -= SPEED;
    }
    if(keys[3]) {
        app->pc.mousePos.y += SPEED;
    }

    // clamp it down finally
    if(app->pc.mousePos.y > app->height) app->pc.mousePos.y = app->height-1;
    else if(app->pc.mousePos.y < 0) app->pc.mousePos.y = 0;
    if(app->pc.mousePos.x > app->width) app->pc.mousePos.x = app->width-1;
    else if(app->pc.mousePos.x < 0) app->pc.mousePos.x = 0;
}

void draw(App *app) {
    // Background white
    rect(0, 0, app->width, app->height, 0xffffffff);

    // Draw points
    point_collection_draw(&app->pc);

    for(int y = 0; y<21; y++)
        for(int x = 0; x<13; x++) {
            switch(cursor[y*13+x]) {
                case 1:
                    plot(app->pc.mousePos.x+x,app->pc.mousePos.y+y, 0x00000000);
                    break;
                case 2:
                    plot(app->pc.mousePos.x+x,app->pc.mousePos.y+y, 0xffffffff);
                    break;
            }
        }
}

void balls(uint64_t w, uint64_t h) {

    App app;
    memset(&app, 0, sizeof(App));
    
    app.width = w;
    app.height = h;
    app.running = 1;

    app.pc.mousePos.x = 50; // app.width / 2.0;
    app.pc.mousePos.y = 50; // app.height / 2.0;
    
    app_init_points(&app);

    for(;;) {
        input(&app);
        draw(&app);
        update(&app);
        flip();
    }
}