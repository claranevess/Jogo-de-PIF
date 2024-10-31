#include "raylib.h"
#include "raymath.h"

#define G 740
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 5
#define MAX_OBSTACLES 10
#define OBSTACLE_SPAWN_TIME 1.0f // Obstáculos gerados a cada 1 segundos
#define OBSTACLE_HORIZONTAL_SPD 100.0f // Velocidade horizontal dos obstáculos

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

typedef struct coin {
    Vector2 position;
    bool collected;
    Color color;
} coin;

typedef struct Obstacle {
    Vector2 position;
    Vector2 speed;
    bool active;
    bool movingLeft;  // Adicionando direção do movimento horizontal
    Color color;
} Obstacle;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength);
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta);
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition);
void DrawObstacles(Obstacle* obstacles, int maxObstacles);
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
        {{ -250, 0, 1000, 400 }, 0, LIGHTGRAY },
        {{ -500, 400, 1500, 200 }, 1, GRAY },
        {{ 300, 200, 400, 10 }, 1, GRAY },
        {{ 250, 300, 100, 10 }, 1, GRAY },
        {{ 650, 300, 100, 10 }, 1, GRAY },
        {{ 250, 100, 100, 10 }, 1, GRAY },      // Nova plataforma esquerda acima da plataforma larga
        {{ 650, 100, 100, 10 }, 1, GRAY },      // Nova plataforma direita acima da plataforma larga
        {{ 450 - 37.5f, 50, 180, 10 }, 1, GRAY }, // Penúltima plataforma
        {{220, -30, 100, 10}, 1, GRAY},
        {{-130, -130, 300, 10}, 1, GRAY}
    };

    int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

    // Posicionando as moedas nas plataformas manualmente
    coin coins[MAX_COINS] = {
        {{ 350, 180 }, false, YELLOW},  // Moeda em cima da plataforma 300,200
        {{ 270, 280 }, false, YELLOW},  // Moeda em cima da plataforma 250,300
        {{ 690, 280 }, false, YELLOW},  // Moeda em cima da plataforma 650,300
        {{ 400, 180 }, false, YELLOW},  // Outra na plataforma maior
        {{ 600, 180 }, false, YELLOW},  // Na plataforma maior
    };
    int coinsLength = MAX_COINS;

    // Obstáculos
    Obstacle obstacles[MAX_OBSTACLES] = { 0 };
    float obstacleSpawnTimer = 0.0f;
    
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

        UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, coinsLength);

        // Atualiza o temporizador e gera obstáculos a cada 2 segundos
        obstacleSpawnTimer += deltaTime;
        if (obstacleSpawnTimer >= OBSTACLE_SPAWN_TIME) {
            obstacleSpawnTimer = 0.0f;
            SpawnObstacle(obstacles, MAX_OBSTACLES, (Vector2) { 400, 100 });
        }

        UpdateObstacles(obstacles, MAX_OBSTACLES, envItems, envItemsLength, deltaTime);

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

        // Desenhar as moedas
        for (int i = 0; i < MAX_COINS; i++) {
            if (!coins[i].collected) {
                DrawCircle(coins[i].position.x, coins[i].position.y, 10, coins[i].color);
            }
        }

        // Desenhar o jogador
        DrawCircle(player.position.x, player.position.y, 20, GREEN);

        // Desenhar os obstáculos
        DrawObstacles(obstacles, MAX_OBSTACLES);

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

// Função para atualizar o jogador
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength)
{
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    float radius = 20.0f;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem* ei = envItems + i;
        Vector2* p = &(player->position);

        if (ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y + radius &&
            ei->rect.y <= p->y + player->speed * delta + radius)
        {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y - radius;
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

    // Colisão do jogador com as moedas
    for (int i = 0; i < coinsLength; i++) {
        if (!coins[i].collected) {
            if (CheckCollisionCircles(player->position, 20, coins[i].position, 10)) {
                coins[i].collected = true;
            }
        }
    }
}
int leftCount = 0;
int rightCount = 0;

// Função para gerar obstáculos
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition) {
    for (int i = 0; i < maxObstacles; i++) {
        if (!obstacles[i].active) {
            obstacles[i].position = spawnPosition;
            obstacles[i].speed = (Vector2){ 0.0f, 0.0f };
            obstacles[i].active = true;
            
            if (leftCount <= rightCount) {
                obstacles[i].movingLeft = true;  // Envia para o lado esquerdo
                leftCount++;  // Incrementa o contador da esquerda
            }
            else {
                obstacles[i].movingLeft = false;  // Envia para o lado direito
                rightCount++;  // Incrementa o contador da direita
            }

            obstacles[i].color = RED;
            break;
        }
    }
}

// Função para atualizar obstáculos
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta) {
    for (int i = 0; i < maxObstacles; i++) {
        if (!obstacles[i].active) continue;

        bool hitObstacle = false;

        // Lógica de física do obstáculo (similar ao jogador)
        obstacles[i].speed.y += G * delta;  // Gravidade
        obstacles[i].position.y += obstacles[i].speed.y * delta;

        // Movimento lateral
        if (obstacles[i].movingLeft) {
            obstacles[i].position.x -= OBSTACLE_HORIZONTAL_SPD * delta;
        }
        else {
            obstacles[i].position.x += OBSTACLE_HORIZONTAL_SPD * delta;
        }

        // Verificar colisão com plataformas usando a base do triângulo
        float baseLeft = obstacles[i].position.x - 10;  // Vértice inferior esquerdo do triângulo
        float baseRight = obstacles[i].position.x + 10;  // Vértice inferior direito do triângulo
        float baseY = obstacles[i].position.y + 20;  // Parte inferior do triângulo

        for (int j = 0; j < envItemsLength; j++) {
            EnvItem* ei = &envItems[j];

            if (ei->blocking &&
                baseRight >= ei->rect.x &&  // Verifica se o triângulo está sobre a plataforma
                baseLeft <= ei->rect.x + ei->rect.width &&
                baseY >= ei->rect.y &&
                baseY <= ei->rect.y + obstacles[i].speed.y * delta)
            {
                hitObstacle = true;
                obstacles[i].speed.y = -PLAYER_JUMP_SPD / 2;  // Obstáculo "pula" para a próxima plataforma
                obstacles[i].position.y = ei->rect.y - 20;  // Coloca a base do triângulo sobre a plataforma

                // Ele continua se movendo na mesma direção após a colisão com a plataforma
                break;
            }
        }

        // Se não colidiu com nenhuma plataforma, ele continua caindo
        if (!hitObstacle && obstacles[i].position.y > 600) {  // Fora da tela
            obstacles[i].active = false;  // Desativa o obstáculo quando cair fora da tela
        }
    }
}

// Função para desenhar os obstáculos
void DrawObstacles(Obstacle* obstacles, int maxObstacles) {
    for (int i = 0; i < maxObstacles; i++) {
        if (obstacles[i].active) {
            // Desenhar o obstáculo como um triângulo vermelho
            Vector2 vertices[3] = {
                {obstacles[i].position.x, obstacles[i].position.y},           // Topo do triângulo
                {obstacles[i].position.x - 10, obstacles[i].position.y + 20},  // Esquerda
                {obstacles[i].position.x + 10, obstacles[i].position.y + 20}   // Direita
            };
            DrawTriangle(vertices[0], vertices[1], vertices[2], RED);
        }
    }
}
