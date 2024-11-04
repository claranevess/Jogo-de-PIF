#include "raylib.h"
#include "raymath.h"

#define G 740
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 5
#define MAX_OBSTACLES 10
#define OBSTACLE_SPAWN_TIME 1.0f // Obstáculos gerados a cada 1 segundo
#define OBSTACLE_HORIZONTAL_SPD 100.0f // Velocidade horizontal dos obstáculos

// Estrutura do jogador
typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
} Player;

// Estrutura do ambiente
typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

// Estrutura das moedas
typedef struct coin {
    Vector2 position;
    bool collected;
    Color color;
} coin;

// Estrutura dos obstáculos
typedef struct Obstacle {
    Vector2 position;
    Vector2 speed;
    bool active;
    bool movingLeft;  // Direção do movimento horizontal
    Color color;
} Obstacle;

// Estados do jogo
typedef enum {
    MENU,
    GAMEPLAY
} GameState;

// Funções principais
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength);
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta);
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition);
void DrawObstacles(Obstacle* obstacles, int maxObstacles);

// Variáveis globais (para simplicidade neste exemplo)
int leftCount = 0;
int rightCount = 0;

int main(void) {
    // Inicialização
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - Game with Menu");

    GameState currentState = MENU;  // Estado inicial: MENU

    Player player = { 0 };
    player.position = (Vector2){ 400, 280 };
    player.speed = 0;
    player.canJump = false;

    // Inicialização das plataformas e do ambiente
    EnvItem envItems[] = {
        {{ -250, 0, 1000, 400 }, 0, LIGHTGRAY },
        {{ -300, 400, 1300, 300 }, 1, GRAY },
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

    // Inicialização das moedas
    coin coins[MAX_COINS] = {
        {{ 350, 180 }, false, YELLOW},  // Moeda na plataforma maior
        {{ 270, 280 }, false, YELLOW},  // Moeda na plataforma esquerda
        {{ 690, 280 }, false, YELLOW},  // Moeda na plataforma direita
        {{ 400, 180 }, false, YELLOW},  // Outra moeda na plataforma maior
        {{ 600, 180 }, false, YELLOW}   // Outra moeda na plataforma maior
    };

    // Inicialização dos obstáculos
    Obstacle obstacles[MAX_OBSTACLES] = { 0 };
    float obstacleSpawnTimer = 0.0f;

    // Configuração da câmera
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Gerenciamento de estados do jogo
        if (currentState == MENU) {
            // Transição para GAMEPLAY ao pressionar Enter
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = GAMEPLAY;
            }
        }
        else if (currentState == GAMEPLAY) {
            // Atualização do jogo
            UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, MAX_COINS);

            // Gerar obstáculos a cada intervalo definido
            obstacleSpawnTimer += deltaTime;
            if (obstacleSpawnTimer >= OBSTACLE_SPAWN_TIME) {
                obstacleSpawnTimer = 0.0f;
                SpawnObstacle(obstacles, MAX_OBSTACLES, (Vector2) { 400, 100 });
            }

            UpdateObstacles(obstacles, MAX_OBSTACLES, envItems, envItemsLength, deltaTime);

            // Atualização da câmera
            camera.target = player.position;
            camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
            else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

            if (IsKeyPressed(KEY_R)) {
                camera.zoom = 1.0f;
                player.position = (Vector2){ 400, 280 };
            }
        }

        // Renderização
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == MENU) {
            // Tela de menu
            DrawText("Bem-vindo ao Plataformia!", screenWidth / 2 - MeasureText("Bem-vindo ao Plataformia!", 20) / 2, screenHeight / 2 - 20, 20, BLACK);
            DrawText("Aperte ENTER para começar", screenWidth / 2 - MeasureText("Aperte ENTER para começar", 20) / 2, screenHeight / 2 + 10, 20, DARKGRAY);
            DrawText("Use as setas do teclado para se mover e a barra de espaço para pular", screenWidth / 2 - MeasureText("Use as setas do teclado para se mover e a barra de espaço para pular", 20) / 2, screenHeight / 2 + 40, 20, DARKGRAY);
        }
        else if (currentState == GAMEPLAY) {
            // Tela de gameplay
            BeginMode2D(camera);

            for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

            // Desenha moedas
            for (int i = 0; i < MAX_COINS; i++) {
                if (!coins[i].collected) {
                    DrawCircle(coins[i].position.x, coins[i].position.y, 10, coins[i].color);
                }
            }

            // Desenha o jogador
            DrawCircle(player.position.x, player.position.y, 20, GREEN);

            // Desenha obstáculos
            DrawObstacles(obstacles, MAX_OBSTACLES);

            EndMode2D();

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
            DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
        }

        EndDrawing();
    }

    // Finalização
    CloseWindow();

    return 0;
}

// Atualiza o jogador
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength) {
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_SPACE) && player->canJump) {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    float radius = 20.0f;

    for (int i = 0; i < envItemsLength; i++) {
        EnvItem* ei = envItems + i;
        Vector2* p = &(player->position);

        if (ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y + radius &&
            ei->rect.y <= p->y + player->speed * delta + radius) {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y - radius;
            break;
        }
    }

    if (!hitObstacle) {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    }
    else {
        player->canJump = true;
    }

    // Colisão do jogador com as moedas
    for (int i = 0; i < coinsLength; i++) {
        if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, 10)) {
            coins[i].collected = true;
        }
    }
}

// Gera novos obstáculos
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition) {
    for (int i = 0; i < maxObstacles; i++) {
        if (!obstacles[i].active) {
            obstacles[i].position = spawnPosition;
            obstacles[i].speed = (Vector2){ 0.0f, 0.0f };
            obstacles[i].active = true;

            if (leftCount <= rightCount) {
                obstacles[i].movingLeft = true;  // Movimento para a esquerda
                leftCount++;
            }
            else {
                obstacles[i].movingLeft = false;  // Movimento para a direita
                rightCount++;
            }

            obstacles[i].color = RED;
            break;
        }
    }
}

// Atualiza os obstáculos
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta) {
    for (int i = 0; i < maxObstacles; i++) {
        if (!obstacles[i].active) continue;

        bool hitObstacle = false;

        obstacles[i].speed.y += G * delta;  // Gravidade
        obstacles[i].position.y += obstacles[i].speed.y * delta;

        if (obstacles[i].movingLeft) {
            obstacles[i].position.x -= OBSTACLE_HORIZONTAL_SPD * delta;
        }
        else {
            obstacles[i].position.x += OBSTACLE_HORIZONTAL_SPD * delta;
        }

        float baseLeft = obstacles[i].position.x - 10;
        float baseRight = obstacles[i].position.x + 10;
        float baseY = obstacles[i].position.y + 20;

        for (int j = 0; j < envItemsLength; j++) {
            EnvItem* ei = &envItems[j];

            if (ei->blocking &&
                baseRight >= ei->rect.x &&
                baseLeft <= ei->rect.x + ei->rect.width &&
                baseY >= ei->rect.y &&
                baseY <= ei->rect.y + obstacles[i].speed.y * delta) {
                hitObstacle = true;
                obstacles[i].speed.y = -PLAYER_JUMP_SPD / 2;
                obstacles[i].position.y = ei->rect.y - 20;
                break;
            }
        }

        if (!hitObstacle && obstacles[i].position.y > 600) {
            obstacles[i].active = false;
        }
    }
}

// Desenha os obstáculos
void DrawObstacles(Obstacle* obstacles, int maxObstacles) {
    for (int i = 0; i < maxObstacles; i++) {
        if (obstacles[i].active) {
            Vector2 vertices[3] = {
                {obstacles[i].position.x, obstacles[i].position.y},
                {obstacles[i].position.x - 10, obstacles[i].position.y + 20},
                {obstacles[i].position.x + 10, obstacles[i].position.y + 20}
            };
            DrawTriangle(vertices[0], vertices[1], vertices[2], RED);
        }
    }
}
