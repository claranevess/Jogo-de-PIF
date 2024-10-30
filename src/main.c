#include "raylib.h"
#include "raymath.h"

#define G 740
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 10

typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

typedef struct coin{
    Vector2 position;
    bool collected;
    Color color;
}coin;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, coin *coinsLenght);
void UpdateCameraCenter(Camera2D* camera, Player* player, EnvItem* envItems, int envItemsLength, float delta, int width, int height);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

    Player player = { 0 };
    player.position = (Vector2){ 400, 280 };
    player.speed = 0;
    player.canJump = false;
    EnvItem envItems[] = {
        {{ -500, 0, 1500, 400 }, 0, LIGHTGRAY },
        {{ -500, 400, 1500, 200 }, 1, GRAY },
        {{ 300, 200, 400, 10 }, 1, GRAY },
        {{ 250, 300, 100, 10 }, 1, GRAY },
        {{ 650, 300, 100, 10 }, 1, GRAY }
    };

    int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

    coin coins[MAX_COINS] = {
        {{ 150, 100 }, false, YELLOW},
        {{ 200, 200 }, false, YELLOW},
        {{ 150, 150 }, false, YELLOW},
        {{ 150, 200 }, false, YELLOW},
        {{ 75, 45 }, false, YELLOW},
        {{ 150, 75 }, false, YELLOW},
        {{ 150, 150 }, false, YELLOW},
        {{ 50, 150 }, false, YELLOW},
        {{ 130, 150 }, false, YELLOW},
        {{ 82, 95 }, false, YELLOW},
    };
    int coinsLenght = MAX_COINS;

    Camera2D camera;
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        float deltaTime = GetFrameTime();
        UpdatePlayer(&player, envItems, envItemsLength, deltaTime, &coins, coinsLenght);
        camera.target = player.position;
        camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2){ 400, 280 };
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(WHITE);


        BeginMode2D(camera);

        for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);
        for (int i = 0; i < MAX_COINS; i++) {
            if (!coins[i].collected){
                DrawCircle(coins[i].position.x, coins[i].position.y, 10, coins[i].color);
            }
        }
        DrawCircle(player.position.x, player.position.y, 20, GREEN);


        EndMode2D();

        DrawText("Controls:", 20, 20, 10, BLACK);
        DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
        DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
        DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);



        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin *coins, coin *coinsLength)
{
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    float radius = 20.0f;  // Raio do círculo do player

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem* ei = envItems + i;
        Vector2* p = &(player->position);

        // Verifica se o círculo do jogador colide com o topo de uma plataforma
        if (ei->blocking &&
            ei->rect.x <= p->x &&  // Verifica se o jogador está dentro da largura da plataforma
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y + radius &&  // Ajusta para o raio do círculo
            ei->rect.y <= p->y + player->speed * delta + radius) // Verifica se o jogador está caindo sobre a plataforma
        {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y - radius;  // Ajusta a posição do jogador para estar em cima da plataforma
            break;
        }
    }

    if (!hitObstacle)
    {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    }
    else
    {
        player->canJump = true;
    }

    // Logica de colisão do jogador com as moedas

    for (int i = 0; i<coinsLength; i++) {
        if (!coins[i].collected) {
            if (CheckCollisionCircles(player->position, 20, coins[i].position, 10)) {
                coins[i].collected = true;
            }
        }
    }
}


void UpdateCameraCenter(Camera2D* camera, Player* player, EnvItem* envItems, int envItemsLength, float delta, int width, int height)
{
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };
    camera->target = player->position;
}