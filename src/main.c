#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>  // Para malloc e free
#include <stdio.h> // arquivo de texto
#include <string.h>  // Para strcpy()

#define G 740
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_HOR_SPD 200.0f
#define MAX_COINS 6
#define OBSTACLE_SPAWN_TIME 1.0f
#define OBSTACLE_HORIZONTAL_SPD 100.0f

int leftCount = 0;
int leftCount2 = 0;
int rightCount = 0;
int rightCount2 = 0;
char playerName[20] = "\0";  // Nome do jogador
int nameIndex = 0;           // Índice para controlar o texto digitado
bool nameEntered = false;    // Flag para saber se o nome foi digitado
char key;
bool save = false;
Texture2D backgroundgameplayy;


typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    int lives;  // Número de vidas do jogador
    bool damaged;  // Novo campo para controlar o dano
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

typedef struct ObstacleNode {
    Obstacle obstacle;
    struct ObstacleNode* next;
} ObstacleNode;

typedef struct {
    char name[20];     // Nome do jogador (com limite de 20 caracteres)
    int coinsCollected; // Número de moedas coletadas
} PlayerRecord;

typedef enum {
    MENU,
    GAMEPLAY,
    GAMEOVER,
    LEADERBOARD
} GameState;

// Funções principais
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected);
void AddObstacle(ObstacleNode** head, Vector2 spawnPosition);
void AddObstacle2(ObstacleNode** head, Vector2 spawnPosition);
void UpdateObstacles(ObstacleNode* head, EnvItem* envItems, int envItemsLength, float delta, Player* player, GameState* currentState);
void RemoveInactiveObstacles(ObstacleNode** head);
void DrawObstacles(ObstacleNode* head);
void FreeObstacleList(ObstacleNode* head);
void DrawHealthBar(Player* player, int screenWidth);  // Função para desenhar a barra de vidas
// Declarações das funções de arquivo
void SavePlayerRecord(const char* filename, PlayerRecord* player);
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);

int ComparePlayerRecords(const void* a, const void* b) {
	PlayerRecord* recordA = (PlayerRecord*)a;
	PlayerRecord* recordB = (PlayerRecord*)b;
	return recordB->coinsCollected - recordA->coinsCollected;  // Ordem decrescente
}

void SavePlayerRecord(const char* filename, PlayerRecord* player) {
	// Carregar registros existentes do arquivo para a memória
	PlayerRecord records[100];
	int recordCount = LoadPlayerRecords(filename, records, 100);

	// Adicionar o novo jogador à lista
	records[recordCount] = *player;
	recordCount++;

	// Ordenar a lista em ordem decrescente de moedas
	qsort(records, recordCount, sizeof(PlayerRecord), ComparePlayerRecords);

	// Sobrescrever o arquivo com a lista ordenada
	FILE* file = fopen(filename, "w");  // "w" para sobrescrever o arquivo

	if (file == NULL) {
		printf("Erro ao abrir o arquivo para escrita.\n");
		return;
	}

	// Escrever todos os registros ordenados no arquivo
	for (int i = 0; i < recordCount; i++) {
		fprintf(file, "%s %d\n", records[i].name, records[i].coinsCollected);
	}

	fclose(file);
}


int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords) {
	FILE* file = fopen(filename, "r");  // Modo de leitura

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
	if (count == 0) {
		DrawText("Nenhum registro encontrado.", screenWidth / 2 - MeasureText("Nenhum registro encontrado.", 20) / 2, 150, 20, DARKGRAY);
		return;
	}

	// Ordena os registros em ordem decrescente usando qsort
	qsort(records, count, sizeof(PlayerRecord), ComparePlayerRecords);

	// Desenha o título da leaderboard
	DrawText("Top Jogadores", screenWidth / 2 - MeasureText("Top Jogadores", 30) / 2, 50, 30, BLACK);

	// Exibe os 5 melhores jogadores na tela
	for (int i = 0; i < count && i < 5; i++) {
		char text[50];
		snprintf(text, sizeof(text), "%d. %s - %d moeda(s)", i + 1, records[i].name, records[i].coinsCollected);
		DrawText(text, screenWidth / 2 - MeasureText(text, 20) / 2, 150 + i * 30, 20, DARKGRAY);
	}

	// Mensagem para voltar ao menu
	DrawText("Pressione ENTER para voltar ao menu", screenWidth / 2 - MeasureText("Pressione ENTER para voltar ao menu", 20) / 2, 400, 20, BLACK);
}

// Funções de manipulação de arquivo
void SavePlayerRecord(const char* filename, PlayerRecord* player);
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);



// Função de atualização do jogador
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected) {
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

	if (player->position.y > 600) {
		player->lives--;
		if (player->lives <= 0) {
			*currentState = GAMEOVER;
		}
		else {
			player->position = (Vector2){ 400, 280 };
			player->speed = 0;
		}
	}

	for (int i = 0; i < coinsLength; i++) {
		if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, 10)) {
			coins[i].collected = true;
			(*coinsCollected)++;
		}
	}
}

// Função para adicionar um obstáculo que se move à esquerda ou à direita
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

// Função de atualização de obstáculos
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
			playerHit = true;
		}

		current = current->next;
	}

	if (!playerHit) {
		player->damaged = false;
	}
}

// Função para remover obstáculos inativos
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

// Função para desenhar obstáculos
void DrawObstacles(ObstacleNode* head) {
	ObstacleNode* current = head;
	while (current != NULL) {
		if (current->obstacle.active) {
			Vector2 vertices[3] = {
				{current->obstacle.position.x, current->obstacle.position.y},
				{current->obstacle.position.x - 10, current->obstacle.position.y + 20},
				{current->obstacle.position.x + 10, current->obstacle.position.y + 20}
			};
			DrawTriangle(vertices[0], vertices[1], vertices[2], RED);
		}
		current = current->next;
	}
}

// Função para liberar memória da lista de obstáculos
void FreeObstacleList(ObstacleNode* head) {
	ObstacleNode* current = head;
	while (current != NULL) {
		ObstacleNode* temp = current;
		current = current->next;
		free(temp);
	}
}

// Função para desenhar a barra de vidas do jogador
void DrawHealthBar(Player* player, int screenWidth) {
	int barX = 20;
	int barY = 20;
	int barSpacing = 10;
	int heartSize = 20;

	for (int i = 0; i < player->lives; i++) {
		DrawRectangle(barX + i * (heartSize + barSpacing), barY, heartSize, heartSize, RED);
	}
}



int main(void) {
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib - Game with Game Over Screen");

	backgroundgameplayy = LoadTexture("bin/Debug/backgroundgameplayy.png");
	if (backgroundgameplayy.id == 0) {
		printf("Erro ao carregar a imagem de fundo!\n");
	}

	GameState currentState = MENU;

	int coinsCollected = 0;  // Contador de moedas coletadas

	Player player = { 0 };
	player.position = (Vector2){ 400, 280 };
	player.speed = 0;
	player.canJump = false;
	player.lives = 3;  // Jogador começa com 3 vidas

	EnvItem envItems[] = {
		{{ -300, 400, 1300, 300 }, 1, GRAY },
		{{ 300, 200, 400, 10 }, 1, GRAY },
		{{ 250, 300, 100, 10 }, 1, GRAY },
		{{ 650, 300, 100, 10 }, 1, GRAY },
		{{ 250, 100, 100, 10 }, 1, GRAY },
		{{ 650, 100, 100, 10 }, 1, GRAY },
		{{ 450 - 37.5f, 50, 180, 10 }, 1, GRAY },
		{{220, -30, 100, 10}, 1, GRAY},
		{{-130, -130, 300, 10}, 1, GRAY}
	};
	int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

	coin coins[MAX_COINS] = {
		{{ 350, 180 }, false, YELLOW},
		{{ 690, 280 }, false, YELLOW},
		{{510, 30}, false, YELLOW},
		{{300, -45}, false, YELLOW}
	};

	ObstacleNode* obstacleList = NULL;
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
			UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, MAX_COINS, &currentState, &coinsCollected);

			obstacleSpawnTimer += deltaTime;
			if (obstacleSpawnTimer >= OBSTACLE_SPAWN_TIME) {
				obstacleSpawnTimer = 0.0f;
				AddObstacle(&obstacleList, (Vector2) { 500, 100 });
				AddObstacle2(&obstacleList, (Vector2) { 500, -100 });
			}

			UpdateObstacles(obstacleList, envItems, envItemsLength, deltaTime, &player, &currentState);
			RemoveInactiveObstacles(&obstacleList);

			camera.target = player.position;
			camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
			if (camera.zoom > 3.0f) camera.zoom = 3.0f;
			else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

			if (IsKeyPressed(KEY_R)) {
				camera.zoom = 1.0f;
				player.position = (Vector2){ 400, 280 };
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
			DrawTexture(backgroundgameplayy, 0, 0, WHITE);

			BeginMode2D(camera);

			for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

			for (int i = 0; i < MAX_COINS; i++) {
				if (!coins[i].collected) {
					DrawCircle(coins[i].position.x, coins[i].position.y, 10, coins[i].color);
				}
			}

			DrawCircle(player.position.x, player.position.y, 20, GREEN);
			DrawObstacles(obstacleList);

			EndMode2D();

			DrawHealthBar(&player, screenWidth);

			// Desenha o contador de moedas na tela
			DrawText(TextFormat("Moedas coletadas: %d", coinsCollected), 20, 50, 20, GOLD);
		}
		else if (currentState == LEADERBOARD) {
			ClearBackground(SKYBLUE);

			// Carregar e exibir os melhores jogadores
			PlayerRecord records[100];
			int recordCount = LoadPlayerRecords("records.txt", records, 100);

			// Exibir os 5 melhores jogadores na tela
			DisplayTopPlayers(records, recordCount, screenWidth);

			// Voltar para o menu se o jogador pressionar ENTER
			if (IsKeyPressed(KEY_ENTER)) {
				currentState = MENU;
				player.position = (Vector2){ 400, 280 };
				player.speed = 0;
				player.lives = 3;
				coinsCollected = 0;
			}
		}


		else if (currentState == GAMEOVER) {
			ClearBackground(RED);
			DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 60, 40, BLACK);
			DrawText("Digite seu nome e pressione ENTER", screenWidth / 2 - 200, screenHeight / 2, 20, DARKGRAY);
			DrawText(playerName, screenWidth / 2 - 100, screenHeight / 2 + 40, 20, BLACK);


			// Capturar entrada do teclado para o nome do jogador
			do {
				key = GetCharPressed();  // Capturar o próximo caractere
				// Somente permitir letras, números e espaços

				if ((key >= 32) && (key <= 125) && (nameIndex < 19)) {
					playerName[nameIndex] = (char)key;
					nameIndex++;
					playerName[nameIndex] = '\0';  // Adicionar o terminador nulo
				}

				// Apagar o último caractere se BACKSPACE for pressionado
				if (IsKeyPressed(KEY_BACKSPACE) && nameIndex > 0) {
					nameIndex--;
					playerName[nameIndex] = '\0';
				}

			} while (key > 0);

			// Quando ENTER é pressionado, salvar o registro e exibir a tela de leaderboard
			if (IsKeyPressed(KEY_ENTER) && nameIndex > 0) {
				PlayerRecord currentPlayer;
				strcpy(currentPlayer.name, playerName);
				currentPlayer.coinsCollected = coinsCollected;

				// Salva o novo registro no arquivo
				SavePlayerRecord("records.txt", &currentPlayer);

				// Carregar os registros atualizados para exibir na tela
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);

				// Mudar para o estado LEADERBOARD
				currentState = LEADERBOARD;

				// Resetar variáveis de entrada
				playerName[0] = '\0';
				nameIndex = 0;
				coinsCollected = 0;

				for (int i = 0; i < MAX_COINS; i++) {
					coins[i].collected = false;
				}
			}


			else if (currentState == LEADERBOARD) {
				ClearBackground(SKYBLUE);

				// Carregar e exibir os melhores jogadores
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);

				// Exibir os 5 melhores jogadores na tela usando DisplayTopPlayers
				DisplayTopPlayers(records, recordCount, screenWidth);

				// Voltar para o menu se o jogador pressionar ENTER
				if (IsKeyPressed(KEY_ENTER)) {
					currentState = MENU;
					player.position = (Vector2){ 400, 280 };
					player.speed = 0;
					player.lives = 3;
					coinsCollected = 0;
				}
			}

			// Quando ENTER é pressionado no estado GAMEOVER para salvar o registro e voltar ao menu
			if (save) {
				PlayerRecord currentPlayer;
				strcpy(currentPlayer.name, playerName);
				currentPlayer.coinsCollected = coinsCollected;

				SavePlayerRecord("records.txt", &currentPlayer);

				// Exibir os melhores jogadores no estado LEADERBOARD
				PlayerRecord records[100];
				int recordCount = LoadPlayerRecords("records.txt", records, 100);

				currentState = LEADERBOARD;

				// Resetar variáveis
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
	CloseWindow();
	return 0;
}