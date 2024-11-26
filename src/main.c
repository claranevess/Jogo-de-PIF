#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "resource_dir.h"

// Constantes para o jogo
#define G 720
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 6
#define OBSTACLE_SPAWN_TIME 1.0f
#define OBSTACLE_HORIZONTAL_SPD 100.0f
#define PLAYER_SCALE 0.4f
#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 40;
#define BOSS_SCALE 0.4f

// Variáveis globais para controle de contagem e estados
int leftCount = 0;
int leftCount2 = 0;
int rightCount = 0;
int rightCount2 = 0;
char playerName[20] = "\0";
int nameIndex = 0;
bool nameEntered = false;
char key;
bool save = false;
int currentMap = 1;
float coinScale = 0.1f;
int frameIndex = 0;
float frameTime = 0.0f;
float frameSpeed = 0.1f;
bool facingRight = true;
float bossSpeed;
bool movingLeft = true;
Music musicadefundo;

// Texturas usadas no jogo
Texture2D backgroundgameplayy;
Texture2D moedas;              
Texture2D concreto;           
Texture2D plataforma;         
Texture2D obstaculo;           
Texture2D playerparado;        
Texture2D playerpulando;      
Texture2D playerdireita[3];   
Texture2D playeresquerda[3]; 
Texture2D vida;               
Texture2D vilao;
Texture2D teia;

// Estruturas do jogo
typedef struct Player {
	Vector2 position;         
	float speed;              
	bool canJump;            
	int lives;                
	bool damaged;             
	bool isJumping;          
	bool dead;
} Player;

typedef struct Boss {
	Vector2 position;
	int health;
	bool active;
	bool defeated;
	Rectangle rect;
} Boss;
typedef struct EnvItem {
	Rectangle rect;           
	int blocking;            
	Color color;             
} EnvItem;
typedef struct coin {
	Vector2 position;
	bool collected;
	float radius;
} coin;
typedef struct Obstacle {
	Vector2 position;        
	Vector2 speed;            
	bool active;             
	bool movingLeft;          
	Color color;
} Obstacle;
typedef struct ObstacleNode {
	Obstacle obstacle;
	struct ObstacleNode* next;
} ObstacleNode;
typedef struct {
	char name[20];
	int coinsCollected;
} PlayerRecord;
typedef enum {
	MENU,
	GAMEPLAY,                
	GAMEOVER,                 
	LEADERBOARD,
	VICTORY
} GameState;
typedef struct Web {
	Vector2 position;
	Vector2 speed;
	bool active;
} Web;


// Declaração das funções principais do jogo
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected);
void AddObstacle(ObstacleNode** head, Vector2 spawnPosition);
void AddObstacle2(ObstacleNode** head, Vector2 spawnPosition);
void AddObstacle3(ObstacleNode** head, Vector2 spawnPosition);
void UpdateObstacles(ObstacleNode* head, EnvItem* envItems, int envItemsLength, float delta, Player* player, GameState* currentState);
void RemoveInactiveObstacles(ObstacleNode** head);
void DrawObstacles(ObstacleNode* head);
void FreeObstacleList(ObstacleNode* head);
void DrawHealthBar(Player* player, int screenWidth);
void SavePlayerRecord(const char* filename, PlayerRecord* player);
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);
void SavePlayerRecord(const char* filename, PlayerRecord* player);

int ComparePlayerRecords(const void* a, const void* b) {
	PlayerRecord* recordA = (PlayerRecord*)a;
	PlayerRecord* recordB = (PlayerRecord*)b;
	return recordB->coinsCollected - recordA->coinsCollected;
}
void SavePlayerRecord(const char* filename, PlayerRecord* player) {
	PlayerRecord records[100];
	int recordCount = LoadPlayerRecords(filename, records, 100);
	records[recordCount] = *player;
	recordCount++;
	qsort(records, recordCount, sizeof(PlayerRecord), ComparePlayerRecords);
	FILE* file = fopen(filename, "w");
	if (file == NULL) {
		printf("Erro ao abrir o arquivo para escrita.\n");
		return;
	}
	for (int i = 0; i < recordCount; i++) {
		fprintf(file, "%s %d\n", records[i].name, records[i].coinsCollected);
	}
	fclose(file);
}
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		printf("Erro ao abrir o arquivo para leitura.\n");
		return 0;
	}
	int count = 0;
	while (count < maxRecords && fscanf(file, "%19s %d", records[count].name, &records[count].coinsCollected) == 2) {
		count++;
	}
	fclose(file);
	return count;
}
void DisplayTopPlayers(PlayerRecord* records, int count, int screenWidth) {
	int screenHeight = GetScreenHeight();
	int baseY = screenHeight / 2 - 100;
	int spacing = 40;
	if (count == 0) {
		DrawText("Nenhum registro encontrado.",
			screenWidth / 2 - MeasureText("Nenhum registro encontrado.", 20) / 2,
			baseY,
			20, WHITE);
		return;
	}
	qsort(records, count, sizeof(PlayerRecord), ComparePlayerRecords);
	DrawText("Top Jogadores",
		screenWidth / 2 - MeasureText("Top Jogadores", 30) / 2,
		baseY - spacing * 2,
		30, WHITE);
	for (int i = 0; i < count && i < 5; i++) {
		char text[50];
		snprintf(text, sizeof(text), "%d. %s - %d moeda(s)", i + 1, records[i].name, records[i].coinsCollected);
		DrawText(text,
			screenWidth / 2 - MeasureText(text, 20) / 2,
			baseY + i * spacing,
			20, WHITE);
	}
	DrawText("Pressione ENTER para voltar ao menu",
		screenWidth / 2 - MeasureText("Pressione ENTER para voltar ao menu", 20) / 2,
		baseY + spacing * 7, 20, WHITE);
}


void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected) {
	float playerAltura = 40.0f;
	if (IsKeyDown(KEY_LEFT)) {
		player->position.x -= PLAYER_HOR_SPD * delta;
		facingRight = false;
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3;
			frameTime = 0.0f;
		}
	}
	else if (IsKeyDown(KEY_RIGHT)) {
		player->position.x += PLAYER_HOR_SPD * delta;
		facingRight = true;
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3;
			frameTime = 0.0f;
		}
	}
	else {
		frameIndex = 0;
	}
	if (IsKeyPressed(KEY_SPACE) && player->canJump) {
		player->speed = -PLAYER_JUMP_SPD;
		player->canJump = false;
		player->isJumping = true;
	}
	player->speed += G * delta;
	player->position.y += player->speed * delta;
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		if (ei->blocking && CheckCollisionPointRec((Vector2) { player->position.x, player->position.y + playerAltura }, ei->rect)) {
			player->canJump = true;
			player->isJumping = false;
			player->speed = 0;
			player->position.y = ei->rect.y - playerAltura + 1;
			break;
		}
	}
	if (player->position.y > 600) {
		player->lives--;
		if (player->lives <= 0) {
			*currentState = GAMEOVER;
		}
		else{
			player->position = (Vector2){ 400, 280 };
			player->speed = 0;
		}
	}
	for (int i = 0; i < coinsLength; i++) {
		float coinRadius = (float)moedas.width / 4;
		if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, coins[i].radius)) {
			coins[i].collected = true;
			(*coinsCollected)++;
		}
	}
}
void AddObstacle(ObstacleNode** head, Vector2 spawnPosition) {
	ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
	if (newNode == NULL) return;
	newNode->obstacle.position = spawnPosition;
	newNode->obstacle.speed = (Vector2){ 0.0f, 0.0f };
	newNode->obstacle.active = true;
	newNode->obstacle.color = RED;
	if (leftCount <= rightCount) {
		newNode->obstacle.movingLeft = true;
		leftCount++;
	}
	else {
		newNode->obstacle.movingLeft = false;
		rightCount++;
	}
	newNode->next = *head;
	*head = newNode;
}

// Função alternativa para adicionar outro tipo de obstáculo
void AddObstacle2(ObstacleNode** head, Vector2 spawnPosition) {
	ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
	if (newNode == NULL) return;
	newNode->obstacle.position = spawnPosition;
	newNode->obstacle.speed = (Vector2){ 0.0f, 0.0f };
	newNode->obstacle.active = true;
	newNode->obstacle.color = RED;
	if (leftCount2 <= rightCount2) {
		newNode->obstacle.movingLeft = true;
		leftCount2++;
	}
	else {
		newNode->obstacle.movingLeft = false;
		rightCount2++;
	}
	newNode->next = *head;
	*head = newNode;
}
void AddObstacle3(ObstacleNode** head, Vector2 spawnPosition) {
	ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
	if (newNode == NULL) return;
	newNode->obstacle.position = spawnPosition;
	newNode->obstacle.speed = (Vector2){ 0.0f, 0.0f };
	newNode->obstacle.active = true;
	newNode->obstacle.color = RED;
	if (leftCount2 <= rightCount2) {
		newNode->obstacle.movingLeft = true;
		leftCount2++;
	}
	else {
		newNode->obstacle.movingLeft = false;
		rightCount2++;
	}
	newNode->next = *head;
	*head = newNode;
}
void UpdateObstacles(ObstacleNode* head, EnvItem* envItems, int envItemsLength, float delta, Player* player, GameState* currentState) {
	ObstacleNode* current = head;
	bool playerHit = false;
	while (current != NULL) {
		if (!current->obstacle.active) {
			current = current->next;
			continue;
		}
		bool hitObstacle = false;
		current->obstacle.speed.y += G * delta;
		current->obstacle.position.y += current->obstacle.speed.y * delta;
		if (current->obstacle.movingLeft) {
			current->obstacle.position.x -= OBSTACLE_HORIZONTAL_SPD * delta;
		}
		else {
			current->obstacle.position.x += OBSTACLE_HORIZONTAL_SPD * delta;
		}
		for (int j = 0; j < envItemsLength; j++) {
			EnvItem* ei = &envItems[j];
			if (ei->blocking &&
				current->obstacle.position.x + 10 >= ei->rect.x &&
				current->obstacle.position.x - 10 <= ei->rect.x + ei->rect.width &&
				current->obstacle.position.y + 20 >= ei->rect.y &&
				current->obstacle.position.y <= ei->rect.y + current->obstacle.speed.y * delta) {
				hitObstacle = true;
				current->obstacle.speed.y = -PLAYER_JUMP_SPD / 2;
				current->obstacle.position.y = ei->rect.y - 20;
				break;
			}
		}
		if (current->obstacle.position.x > 850 || current->obstacle.position.x < -50) {
			current->obstacle.active = false;
		}
		if (!hitObstacle && current->obstacle.position.y > 600) {
			current->obstacle.active = false;
		}
		float playerRadius = 20.0f;
		if (CheckCollisionCircles(player->position, playerRadius, current->obstacle.position, 10.0f)) {
			if (!player->damaged) {
				player->lives--;
				player->damaged = true;

				if (player->lives <= 0) {
					*currentState = GAMEOVER;
					return;
				}
			}
			current->obstacle.active = false;
			playerHit = true;
		}
		current = current->next;
	}
	if (!playerHit) {
		player->damaged = false;
	}
}

void UpdateBoss(Boss* boss, Player* player, EnvItem* envItems, int envItemsLength, float deltaTime, GameState* currentState) {
	if (boss->active && !boss->defeated) {
		float platformLeft = envItems[envItemsLength - 1].rect.x;
		float platformRight = envItems[envItemsLength - 1].rect.x + envItems[envItemsLength - 1].rect.width;
		float bossSpeed = 550.0f;
		float bossWidth = 50.0f;
		static bool movingLeft = true;
		if (movingLeft) {
			boss->position.x -= bossSpeed * deltaTime;
			if (boss->position.x <= platformLeft) {
				movingLeft = false;
			}
		}
		else {
			boss->position.x += bossSpeed * deltaTime;
			if (boss->position.x + bossWidth >= platformRight) {
				movingLeft = true;
			}
		}
		Rectangle bossRect = { boss->position.x, boss->position.y - 20, bossWidth, 60 };
		if (CheckCollisionPointRec(player->position, bossRect)) {
			if (!player->damaged) {
				player->lives--;
				player->damaged = true;
				if (player->lives <= 0) {
					*currentState = GAMEOVER;
					return;
				}
			}
		}
		else {
			player->damaged = false;
		}
	}
}
void RemoveInactiveObstacles(ObstacleNode** head) {
	ObstacleNode* current = *head;
	ObstacleNode* prev = NULL;
	while (current != NULL) {
		if (!current->obstacle.active) {
			if (prev == NULL) {
				*head = current->next;
			}
			else {
				prev->next = current->next;
			}
			ObstacleNode* temp = current;
			current = current->next;
			free(temp);
		}
		else {
			prev = current;
			current = current->next;
		}
	}
}
void DrawObstacles(ObstacleNode* head) {
	ObstacleNode* current = head;
	while (current != NULL) {
		if (current->obstacle.active) {
			DrawTexturePro(
				obstaculo,
				(Rectangle) {
				0, 0, obstaculo.width, obstaculo.height
			},
				(Rectangle) {
				current->obstacle.position.x - 20, current->obstacle.position.y - 20, 30, 30
			},
				(Vector2) {
				20, 20
			},
				0.0f,
				WHITE
			);
		}
		current = current->next;
	}
}
void FreeObstacleList(ObstacleNode* head) {
	ObstacleNode* current = head;
	while (current != NULL) {
		ObstacleNode* temp = current;
		current = current->next;
		free(temp);
	}
}
void DrawHealthBar(Player* player, int screenWidth) {
	int heartSize = 30;
	int barX = 20;
	int barY = 50;
	int heartSpacing = 10;
	for (int i = 0; i < player->lives; i++) {
		DrawTextureEx(vida, (Vector2) { barX + i * (heartSize + heartSpacing), barY }, 0.0f, (float)heartSize / vida.width, WHITE);
	}
}

void DrawBoss(Boss* boss) {
	if (boss->active && !boss->defeated) {
		DrawRectangle(boss->position.x - 0, boss->position.y - 20, 30, 40, RED);
		DrawText(TextFormat("Vida: %d", boss->health), boss->position.x - 30, boss->position.y - 50, 20, BLACK);
		DrawTextureEx(vilao, (Vector2) { boss->position.x - 30, boss->position.y - 40 }, 0.0f, BOSS_SCALE, WHITE);
	}
}

void UpdateWeb(Web* web, Boss* boss, float deltaTime, GameState* currentState) {
	if (web->active) {
		web->position.x += web->speed.x * deltaTime;
		Rectangle bossRect = { boss->position.x, boss->position.y - 20, 50, 60 };
		if (CheckCollisionPointRec(web->position, bossRect)) {
			web->active = false;
			boss->health -= 20;
			if (boss->health <= 0) {
				boss->defeated = true;
				boss->active = false;
				*currentState = VICTORY;
				boss->defeated = false;
				boss->active = true;
				boss->health = 100;
			}
		}
		if (web->position.x < 0 || web->position.x > 1920) {
			web->active = false;
		}
	}
}

// Função principal do programa
int main(void) {

	const int screenWidth = 1920;
	const int screenHeight = 1080; 
	InitWindow(screenWidth, screenHeight, "Plataformia - a Spider-Man Game");

	InitAudioDevice();
	SearchAndSetResourceDir("resources");
	
	PlayMusicStream(musicadefundo);

	Web web = { 0 };
	web.active = false;
	web.speed = (Vector2){ 500.0f, 0.0f };

	// Carregamento das texturas
	
	backgroundgameplayy = LoadTexture("backgroundgameplayy.png");
	float scale = (float)screenWidth / backgroundgameplayy.width;
	moedas = LoadTexture("coin.png");
	concreto = LoadTexture("concreto.png");
	plataforma = LoadTexture("plataforma.png");
	obstaculo = LoadTexture("obstaculo.png");
	teia = LoadTexture("teia.png");
	playerparado = LoadTexture("playerparado.png");
	playerpulando = LoadTexture("playerpulando.png");
	playerdireita[0] = LoadTexture("playerdireta1.png");
	playerdireita[1] = LoadTexture("playerdireta2.png");
	playerdireita[2] = LoadTexture("playerdireta3.png");
	playeresquerda[0] = LoadTexture("playeresquerda1.png");
	playeresquerda[1] = LoadTexture("playeresquerda2.png");
	playeresquerda[2] = LoadTexture("playeresquerda3.png");
	vilao = LoadTexture("boss.png");
	vida = LoadTexture("vida.png");

	GameState currentState = MENU;
	int coinsCollected = 0;

	// Definição dos itens do ambiente
	EnvItem envItems[] = {
		{{ -300, 400, 1300, 472 }, 1, BLACK },
		{{ 300, 200, 400, 10 }, 1, BLACK },
		{{ 200, 300, 100, 10 }, 1, BLACK },
		{{ 650, 300, 100, 10 }, 1, BLACK },
		{{ 250, 100, 100, 10 }, 1, BLACK },
		{{ 650, 100, 100, 10 }, 1, BLACK },
		{{ 450 - 37.5f, 50, 180, 10 }, 1, BLACK },
		{{ 220, -30, 200, 10 }, 1, BLACK },
		{{600, -50, 100, 10}, 1,BLACK },
		{{ -130, -130, 300, 10 }, 1, BLACK },
		{{-50,-230,100,10}, 1, BLACK},
		{{150, -280, 100, 10}, 1, BLACK},
		{{250, -390, 100, 10}, 1, BLACK},
		{{300, -490, 100, 10}, 1, BLACK},
		{{500, -160, 100, 10}, 1, BLACK},
		{{500, -590, 100, 10}, 1, BLACK},
		{{600, -450, 100, 10}, 1, BLACK},
		{{650, -270,100,10},1,BLACK},
		//boss
		{{-100, -690, 600, 10}, 1, BLACK}
	};
	int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

	// Inicialização do jogador
	Player player = { 0 };
	player.position = (Vector2){ 400, envItems[0].rect.y };
	player.speed = 0;
	player.canJump = true;
	player.lives = 3;
	player.damaged = false;
	player.isJumping = false;
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		if (ei->blocking && CheckCollisionPointRec(player.position, ei->rect)) {
			player.canJump = true;
			player.speed = 0;
			player.position.y = ei->rect.y;
			break;
		}
	}

	Boss boss = { 0 };
	boss.position = (Vector2){ envItems[envItemsLength - 1].rect.x + envItems[envItemsLength - 1].rect.width / 2, envItems[envItemsLength - 1].rect.y - 50 };
	boss.health = 100;
	boss.active = true;
	boss.defeated = false;
	DrawRectangle(boss.position.x, boss.position.y, 50, 50, RED);

	float coinRadius = (moedas.width * coinScale) / 2.0f;
	coin coins[MAX_COINS] = {
		{{ 350, 180 }, false, coinRadius },
		{{400, -600}, false, coinRadius },
		{{ 690, 280 }, false, coinRadius },
		{{ 260, 260 }, false, coinRadius },
		{{ 500, -400 }, false, coinRadius },
		{{ 510, 30 }, false, coinRadius }
	};

	ObstacleNode* obstacleList = NULL;
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
		UpdateMusicStream(musicadefundo);

		if (currentState == MENU) {
			if (IsKeyPressed(KEY_ENTER)) {
				currentState = GAMEPLAY;
			}
		}
		else if (currentState == GAMEPLAY) {
			UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, MAX_COINS, &currentState, &coinsCollected);
			if (IsKeyPressed(KEY_T) && !web.active) {
				web.position = player.position;
				web.active = true;
				if (facingRight) {
					web.speed.x = 500.0f;
				}
				else {
					web.speed.x = -500.0f;
				}
			}
			UpdateBoss(&boss, &player, envItems, envItemsLength, deltaTime, &currentState);
			UpdateWeb(&web, &boss, deltaTime, &currentState);

			obstacleSpawnTimer += deltaTime;
			if (obstacleSpawnTimer >= OBSTACLE_SPAWN_TIME) {
				obstacleSpawnTimer = -2.0f;
				AddObstacle(&obstacleList, (Vector2) { 550, 100 });
				AddObstacle2(&obstacleList, (Vector2) { 500, -100 });
				AddObstacle3(&obstacleList, (Vector2) { -15, -420 });
			}
			UpdateObstacles(obstacleList, envItems, envItemsLength, deltaTime, &player, &currentState);
			RemoveInactiveObstacles(&obstacleList);

			camera.target = player.position;
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);
		if (currentState == MENU) {
			ClearBackground(BLACK);

			int fontSize = 30;
			DrawText("Bem-vindo(a) ao Plataformia!", screenWidth / 2 - MeasureText("Bem-vindo ao Plataformia!", fontSize) / 2, screenHeight / 2 - 200, fontSize, WHITE);
			int introX = screenWidth / 4;
			DrawText("Ano: 2002, Cidade de Nova York.", introX - 150, screenHeight / 2 - 100, fontSize, WHITE);
			DrawText("O jovem Peter Parker, nosso icônico Homem-Aranha,", introX - 300, screenHeight / 2 - 70, fontSize, WHITE);
			DrawText("enfrenta seu maior desafio até agora.", introX - 300, screenHeight / 2 - 40, fontSize, WHITE);
			DrawText("Em uma perseguição pelos arranha-céus de Manhattan,", introX - 300, screenHeight / 2 - 10, fontSize, WHITE);
			DrawText("o perigoso e imprevisível Duende Verde está determinado", introX - 300, screenHeight / 2 + 20, fontSize, WHITE);
			DrawText("a destruir o herói da cidade.", introX - 300, screenHeight / 2 + 50, fontSize, WHITE);

			DrawText("Como jogador, você deve guiar o Homem-Aranha", introX - 300, screenHeight / 2 + 80, fontSize, WHITE);
			DrawText("em uma corrida frenética pelos telhados de Nova York.", introX - 300, screenHeight / 2 + 110, fontSize, WHITE);
			DrawText("Será que você consegue escapar das armadilhas", introX - 300, screenHeight / 2 + 140, fontSize, WHITE);
			DrawText("do Duende Verde e salvar Nova York do caos?", introX - 300, screenHeight / 2 + 170, fontSize, WHITE);

			int commandX = screenWidth * 3 / 4;
			DrawText("Aperte ENTER para começar", commandX - MeasureText("Aperte ENTER para começar", fontSize) / 2, screenHeight / 2 + 10, fontSize, WHITE);
			DrawText("ou ESC para sair.", commandX - MeasureText("ou ESC para sair.", fontSize) / 2, screenHeight / 2 + 40, fontSize, WHITE);
			DrawText("Use as setas do teclado para se mover", commandX - MeasureText("Use as setas do teclado para se mover", fontSize) / 2, screenHeight / 2 + 70, fontSize, WHITE);
			DrawText("Use a tecla T para soltar teia", commandX - MeasureText("Use a tecla T para soltar teia", fontSize) / 2, screenHeight / 2 + 100, fontSize, WHITE);
			DrawText("e a barra de espaço para pular!", commandX - MeasureText("e a barra de espaço para pular!", fontSize) / 2, screenHeight / 2 + 130, fontSize, WHITE);
		}
		else if (currentState == GAMEPLAY) {
			float scaleX = (float)screenWidth / backgroundgameplayy.width * 1.2f;
			float scaleY = (float)screenHeight / backgroundgameplayy.height * 1.2f;
			float parallaxX = -camera.target.x * 0.1f;
			float parallaxY = -camera.target.y * 0.05f - 30;

			DrawTextureEx(backgroundgameplayy, (Vector2) { parallaxX, parallaxY }, 0.0f, scaleX > scaleY ? scaleX : scaleY, WHITE);

			BeginMode2D(camera);

			for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);
			for (int i = 0; i < envItemsLength; i++) {
				if (i == 0) {
					DrawTexturePro(
						concreto,
						(Rectangle) {
						0, 0, concreto.width, concreto.height
					},
						(Rectangle) {
						envItems[i].rect.x, envItems[i].rect.y, envItems[i].rect.width, envItems[i].rect.height
					},
						(Vector2) {
						0, 0
					},
						0.0f,
						WHITE
					);
				}
				else {
					DrawTexturePro(
						plataforma,
						(Rectangle) {
						0, 0, plataforma.width, plataforma.height
					},
						(Rectangle) {
						envItems[i].rect.x, envItems[i].rect.y, envItems[i].rect.width, envItems[i].rect.height
					},
						(Vector2) {
						0, 0
					},
						0.0f,
						WHITE
					);
				}
				DrawBoss(&boss);
			}
			for (int i = 0; i < MAX_COINS; i++) {
				if (!coins[i].collected) {
					Vector2 drawPosition = { coins[i].position.x - (moedas.width * coinScale) / 2, coins[i].position.y - (moedas.height * coinScale) / 2 };
					DrawTextureEx(moedas, drawPosition, 0.0f, coinScale, WHITE);
				}
			}

			if (player.isJumping) {
				DrawTextureEx(playerpulando, (Vector2) { player.position.x - (playerpulando.width * PLAYER_SCALE) / 2, player.position.y - (playerpulando.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_LEFT)) {
				DrawTextureEx(playeresquerda[frameIndex], (Vector2) { player.position.x - (playeresquerda[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playeresquerda[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_RIGHT)) {
				DrawTextureEx(playerdireita[frameIndex], (Vector2) { player.position.x - (playerdireita[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playerdireita[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else {
				DrawTextureEx(playerparado, (Vector2) { player.position.x - (playerparado.width * PLAYER_SCALE) / 2, player.position.y - (playerparado.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}

			if (web.active) {
				DrawTextureEx(teia, web.position, 0.0f, 0.2f, WHITE);
			}

			DrawObstacles(obstacleList);
			EndMode2D();
			DrawHealthBar(&player, screenWidth);
			DrawText(TextFormat("Moedas coletadas: %d", coinsCollected), 20, 80, 20, BLACK);
		}
		else if (currentState == LEADERBOARD) {
			ClearBackground(BLACK);
			PlayerRecord records[100];
			int recordCount = LoadPlayerRecords("records.txt", records, 100);
			DisplayTopPlayers(records, recordCount, screenWidth);
			if (IsKeyPressed(KEY_ENTER)) {
				currentState = MENU;
				player.position = (Vector2){ 400, 280 };
				player.speed = 0;
				player.lives = 3;
				coinsCollected = 0;
				for (int i = 0; i < MAX_COINS; i++) {
					coins[i].collected = false;
				}
			}
		}

		else if (currentState == VICTORY) {
			ClearBackground(BLACK);
			int fontSize = 40;
			DrawText("Parabéns, você venceu o Duende Verde!",
				screenWidth / 2 - MeasureText("Parabéns, você venceu o Duende Verde!", fontSize) / 2,
				screenHeight / 2 - 50, fontSize, WHITE);
			DrawText("Digite seu nome e pressione ENTER", screenWidth / 2 - 200, screenHeight / 2, 20, WHITE);
			DrawText(playerName, screenWidth / 2 - 100, screenHeight / 2 + 40, 20, WHITE);
			key = GetCharPressed();
			if ((key >= 32) && (key <= 125) && (nameIndex < 19)) {
				playerName[nameIndex] = (char)key;
				nameIndex++;
				playerName[nameIndex] = '\0';
			}

			if (IsKeyPressed(KEY_BACKSPACE) && nameIndex > 0) {
				nameIndex--;
				playerName[nameIndex] = '\0';
			}
			if (IsKeyPressed(KEY_ENTER) && nameIndex > 0) {
				PlayerRecord currentPlayer;
				strcpy(currentPlayer.name, playerName);
				currentPlayer.coinsCollected = coinsCollected;

				SavePlayerRecord("records.txt", &currentPlayer);
				currentState = LEADERBOARD;
				playerName[0] = '\0';
				nameIndex = 0;
			}
		}
		else if (currentState == GAMEOVER) {
			ClearBackground(BLACK); 
			DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 60, 40, WHITE);
			DrawText("Digite seu nome e pressione ENTER", screenWidth / 2 - 200, screenHeight / 2, 20, WHITE);
			DrawText(playerName, screenWidth / 2 - 100, screenHeight / 2 + 40, 20, WHITE);
			do {
				key = GetCharPressed();
				if ((key >= 32) && (key <= 125) && (nameIndex < 19)) {
					playerName[nameIndex] = (char)key;
					nameIndex++;
					playerName[nameIndex] = '\0';
				}
				if (IsKeyPressed(KEY_BACKSPACE) && nameIndex > 0) {
					nameIndex--;
					playerName[nameIndex] = '\0';
				}

			} while (key > 0);
			if (IsKeyPressed(KEY_ENTER) && nameIndex > 0) {
				PlayerRecord currentPlayer;
				strcpy(currentPlayer.name, playerName);
				currentPlayer.coinsCollected = coinsCollected;
				SavePlayerRecord("records.txt", &currentPlayer);
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);
				currentState = LEADERBOARD;
				playerName[0] = '\0';
				nameIndex = 0;
				coinsCollected = 0;
				for (int i = 0; i < MAX_COINS; i++) {
					coins[i].collected = false;
				}
			}else if (currentState == LEADERBOARD) {
				ClearBackground(BLACK);
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);
				DisplayTopPlayers(records, recordCount, screenWidth);
				if (IsKeyPressed(KEY_ENTER)) {
					currentState = MENU;
					player.position = (Vector2){ 400, 280 };
					player.speed = 0;
					player.lives = 3;
					coinsCollected = 0;
				}
			}
			if (save) {
				PlayerRecord currentPlayer;
				strcpy(currentPlayer.name, playerName);
				currentPlayer.coinsCollected = coinsCollected;
				SavePlayerRecord("records.txt", &currentPlayer);
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);
				currentState = LEADERBOARD;
				player.position = (Vector2){ 400, 280 };
				player.speed = 0;
				player.lives = 3;
				coinsCollected = 0;
				nameEntered = false;
				playerName[0] = '\0';
				nameIndex = 0;
				save = false;
			}
		}
		EndDrawing();
	}
	FreeObstacleList(obstacleList);
	UnloadTexture(backgroundgameplayy);
	UnloadMusicStream(musicadefundo);
	CloseAudioDevice();
	UnloadTexture(moedas);
	UnloadTexture(concreto);
	UnloadTexture(plataforma);
	UnloadTexture(obstaculo);
	UnloadTexture(playerparado);
	UnloadTexture(playerpulando);
	UnloadTexture(vilao);
	for (int i = 0; i < 3; i++) {
		UnloadTexture(playerdireita[i]);
		UnloadTexture(playeresquerda[i]);
	}
	UnloadTexture(vida);
	UnloadTexture(vilao);
	UnloadTexture(teia);
	CloseWindow();
	return 0;
}
