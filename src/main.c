#include "raylib.h"
#include "raymath.h"

#define G 740
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 5
#define MAX_OBSTACLES 10
#define OBSTACLE_SPAWN_TIME 1.0f
#define OBSTACLE_HORIZONTAL_SPD 100.0f

int leftCount = 0;
int rightCount = 0;

typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    int lives;  // Número de vidas do jogador
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
    bool movingLeft;
    Color color;
} Obstacle;

typedef enum {
    MENU,
    GAMEPLAY,
    GAMEOVER
} GameState;

// Funções principais
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, Obstacle* obstacles, int maxObstacles);
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta);
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition);
void DrawObstacles(Obstacle* obstacles, int maxObstacles);
void DrawHealthBar(Player* player, int screenWidth);  // Nova função para desenhar a barra de vidas

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - Game with Game Over Screen");

    GameState currentState = MENU;

    Player player = { 0 };
    player.position = (Vector2){ 400, 280 };
    player.speed = 0;
    player.canJump = false;
    player.lives = 9;  // Jogador começa com 3 vidas

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

    coin coins[MAX_COINS] = {
        {{ 350, 180 }, false, YELLOW},
        {{ 270, 280 }, false, YELLOW},
        {{ 690, 280 }, false, YELLOW},
        {{ 400, 180 }, false, YELLOW},
        {{ 600, 180 }, false, YELLOW}
    };

    Obstacle obstacles[MAX_OBSTACLES] = { 0 };
    float obstacleSpawnTimer = 0.0f;

    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (currentState == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = GAMEPLAY;
            }
        }
        else if (currentState == GAMEPLAY) {
            UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, MAX_COINS, &currentState, obstacles, MAX_OBSTACLES);



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

            if (IsKeyPressed(KEY_R)) {
                camera.zoom = 1.0f;
                player.position = (Vector2){ 400, 280 };
            }
        }
        else if (currentState == GAMEOVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = MENU;  // Volta ao menu
                player.position = (Vector2){ 400, 280 };  // Reinicia o jogador
                player.speed = 0;
                player.lives = 3;  // Reinicia as vidas
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == MENU) {
            ClearBackground(SKYBLUE);
            DrawText("Bem-vindo ao Plataformia!", screenWidth / 2 - MeasureText("Bem-vindo ao Plataformia!", 20) / 2, screenHeight / 2 - 20, 20, BLACK);
            DrawText("Aperte ENTER para começar", screenWidth / 2 - MeasureText("Aperte ENTER para começar", 20) / 2, screenHeight / 2 + 10, 20, DARKGRAY);
            DrawText("Use as setas do teclado para se mover e a barra de espaço para pular", screenWidth / 2 - MeasureText("Use as setas do teclado para se mover e a barra de espaço para pular", 20) / 2, screenHeight / 2 + 40, 20, DARKGRAY);
        }
        else if (currentState == GAMEPLAY) {
            ClearBackground(DARKGRAY);

            BeginMode2D(camera);

            for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

            for (int i = 0; i < MAX_COINS; i++) {
                if (!coins[i].collected) {
                    DrawCircle(coins[i].position.x, coins[i].position.y, 10, coins[i].color);
                }
            }

            DrawCircle(player.position.x, player.position.y, 20, GREEN);
            DrawObstacles(obstacles, MAX_OBSTACLES);

            EndMode2D();

            DrawHealthBar(&player, screenWidth);  // Desenha a barra de vidas
        }
        else if (currentState == GAMEOVER) {
            ClearBackground(RED);
            DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 60, 40, BLACK);
            DrawText("Aperte ENTER para voltar para o menu", screenWidth / 2 - MeasureText("Aperte ENTER para voltar para o menu", 20) / 2, screenHeight / 2, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Atualiza os obstáculos
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, Obstacle* obstacles, int maxObstacles){
    // Movimento do jogador
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_SPACE) && player->canJump) {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    float radius = 20.0f;  // Raio de colisão do jogador

    // Verifica colisão com as plataformas
    for(int i = 0; i < envItemsLength; i++) {
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

    // Colisão com os obstáculos
    for (int i = 0; i < maxObstacles; i++) {
        if (obstacles[i].active) {
            float distX = player->position.x - obstacles[i].position.x;
            float distY = player->position.y - obstacles[i].position.y;

            // Se a distância entre o jogador e o obstáculo for pequena (indicando colisão)
            if (fabs(distX) < 20.0f && fabs(distY) < 20.0f) {
                hitObstacle = true;  // O jogador colidiu com o obstáculo
                player->lives--;  // O jogador perde uma vida

                if (player->lives <= 0) {
                    *currentState = GAMEOVER;  // Game Over se as vidas acabarem
                }
                else {
                    // Reposiciona o jogador após a colisão
                    player->position = (Vector2){ 400, 280 };
                    player->speed = 0.0f;
                }
            }
        }
    }

    // Colisão do jogador com as moedas
    for (int i = 0; i < coinsLength; i++) {
        if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, 10)) {
            coins[i].collected = true;
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
void SpawnObstacle(Obstacle* obstacles, int maxObstacles, Vector2 spawnPosition) {
    for (int i = 0; i < maxObstacles; i++) {
        if (!obstacles[i].active) {  // Procura por um obstáculo inativo
            obstacles[i].active = true;
            obstacles[i].position = spawnPosition;  // Define a posição do obstáculo
            obstacles[i].speed = (Vector2){ 0, OBSTACLE_HORIZONTAL_SPD };  // Atribui uma velocidade ao obstáculo (exemplo)
            obstacles[i].movingLeft = false;
            obstacles[i].color = RED;  // Cor do obstáculo
            break;  // Interrompe o loop após ativar um obstáculo
        }
    }
}
void UpdateObstacles(Obstacle* obstacles, int maxObstacles, EnvItem* envItems, int envItemsLength, float delta) {
    for (int i = 0; i < maxObstacles; i++) {
        if (obstacles[i].active) {
            // Atualize a posição do obstáculo com base em sua velocidade
            obstacles[i].position.x += obstacles[i].speed.x * delta;
            obstacles[i].position.y += obstacles[i].speed.y * delta;

            // Se o obstáculo sair da tela, desative-o
            if (obstacles[i].position.x < 0 || obstacles[i].position.x > 800 || obstacles[i].position.y > 450) {
                obstacles[i].active = false;
            }
        }
    }
}

// Função para desenhar a barra de vidas
void DrawHealthBar(Player* player, int screenWidth) {
    int barX = 20;  // Posição X da barra
    int barY = 20;  // Posição Y da barra
    int barSpacing = 10;  // Espaço entre os corações
    int heartSize = 20;  // Tamanho de cada coração

    for (int i = 0; i < player->lives; i++) {
        DrawRectangle(barX + i * (heartSize + barSpacing), barY, heartSize, heartSize, RED);  // Desenha um coração por vida
    }
}
