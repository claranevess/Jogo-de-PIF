#include <raylib.h>

#define SCREEN_WIDTH 800    
#define SCREEN_HEIGHT 450    
#define TILE_SIZE 32        
#define NUM_TILES 10         

typedef struct Tile {
    Vector2 position;      
    bool isSolid;          
} Tile;

void DrawCityBackground() {
    DrawRectangle(50, 200, 80, 250, DARKGRAY);  
    DrawRectangle(200, 150, 100, 300, GRAY);   
    DrawRectangle(400, 100, 120, 350, DARKGRAY); 
    DrawRectangle(600, 180, 90, 270, GRAY);     
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo - Inspirado em Nova York");

    Tile tiles[NUM_TILES] = {
        {{150, 380}, true}, {{250, 340}, true}, {{400, 300}, true},
        {{550, 260}, true}, {{700, 220}, true}, {{300, 200}, true},
        {{100, 150}, true}, {{200, 120}, true}, {{350, 100}, true},
        {{500, 80}, true}
    };

    SetTargetFPS(60);  

    while (!WindowShouldClose()) {
        BeginDrawing();                  
        ClearBackground(RAYWHITE);      

        DrawCityBackground();

        for (int i = 0; i < NUM_TILES; i++) {
            if (tiles[i].isSolid) {
                DrawRectangle(tiles[i].position.x, tiles[i].position.y, TILE_SIZE * 2, TILE_SIZE / 2, DARKGRAY);
                DrawRectangle(tiles[i].position.x, tiles[i].position.y + TILE_SIZE / 2, TILE_SIZE * 2, TILE_SIZE / 2, GRAY);
            }
        }

        EndDrawing();  
    }

    CloseWindow();  

    return 0;
}