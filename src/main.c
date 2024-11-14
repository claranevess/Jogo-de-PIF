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
#define PLAYER_SCALE 0.4f  // Ajuste a escala conforme necessário


int leftCount = 0;
int leftCount2 = 0;
int rightCount = 0;
int rightCount2 = 0;
char playerName[20] = "\0";  // Nome do jogador
int nameIndex = 0;           // Índice para controlar o texto digitado
bool nameEntered = false;    // Flag para saber se o nome foi digitado
char key;
bool save = false;
int currentMap = 1; // Variável de controle de mapa
float coinScale = 0.1f; // Escala da moeda
int frameIndex = 0;       // Índice para a animação de corrida
float frameTime = 0.0f;   // Tempo para trocar o frame
float frameSpeed = 0.1f;  // Velocidade da animação
bool facingRight = true;  // Direção em que o player está olhando
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


typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    int lives;  // Número de vidas do jogador
    bool damaged;  // Novo campo para controlar o dano
	bool isJumping;
} Player;

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

//Função para alterar a fase
void TransitionMap(GameState* currentState, int* currentMap, int nextMap) {
	float fadeOutDuration = 1.0f;  // Tempo para o fade out
	float fadeInDuration = 1.0f;   // Tempo para o fade in
	float timer = 0.0f;

	// Iniciar fade out
	while (timer < fadeOutDuration) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Mudando de mapa...", 360, 220, 20, WHITE);
		EndDrawing();
		timer += GetFrameTime();
	}

	*currentMap = nextMap;  // Mudança para o próximo mapa

	// Resetar timer e iniciar fade in
	timer = 0.0f;
	while (timer < fadeInDuration) {
		float alpha = (1.0f - (timer / fadeInDuration)) * 255;
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Carregando mapa...", 360, 220, 20, Fade(WHITE, alpha));
		EndDrawing();
		timer += GetFrameTime();
	}

	*currentState = GAMEPLAY;  // Voltar ao estado de jogo
}

// Função de atualização do jogador
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected) {
	// Altura aproximada do player (ajuste conforme necessário para seu asset)
	float playerAltura = 40.0f;  // Defina de acordo com a altura real do asset

	// Movimentação horizontal do player
	if (IsKeyDown(KEY_LEFT)) {
		player->position.x -= PLAYER_HOR_SPD * delta;
		facingRight = false;

		// Atualizar animação
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3; // Ciclar entre os frames
			frameTime = 0.0f;
		}
	}
	else if (IsKeyDown(KEY_RIGHT)) {
		player->position.x += PLAYER_HOR_SPD * delta;
		facingRight = true;

		// Atualizar animação
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3; // Ciclar entre os frames
			frameTime = 0.0f;
		}
	}
	else {
		// Resetar animação se o player estiver parado
		frameIndex = 0;
	}

	// Lógica de pulo
	if (IsKeyPressed(KEY_SPACE) && player->canJump) {
		player->speed = -PLAYER_JUMP_SPD;  // Impulso para pulo
		player->canJump = false;           // Impede novo pulo no ar
		player->isJumping = true;          // Define estado de pulo
	}

	// Aplicar gravidade
	player->speed += G * delta;
	player->position.y += player->speed * delta;

	// Verificar colisão com o chão e posicionar o player na borda superior da plataforma
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		// Verificar se a base do player está colidindo com a plataforma
		if (ei->blocking && CheckCollisionPointRec((Vector2) { player->position.x, player->position.y + playerAltura }, ei->rect)) {
			player->canJump = true;        // Permite pulo novamente
			player->isJumping = false;     // Define que não está mais no ar
			player->speed = 0;             // Zera a velocidade vertical

			// Ajusta o player para ficar exatamente na borda superior da plataforma
			player->position.y = ei->rect.y - playerAltura + 1; // Ajuste de 1 unidade para alinhar visualmente
			if (ei->rect.y == -130)
			{
				TransitionMap(currentState, &currentMap, currentMap + 1);
			}
			break;
		}
	}

	// Condição para quando o player cai fora da tela
	if (player->position.y > 600) {
		player->lives--;
		if (player->lives <= 0) {
			*currentState = GAMEOVER;
		}
		else {
			// Reinicia a posição do player no centro da tela
			player->position = (Vector2){ 400, 280 };
			player->speed = 0;
		}
	}

	// Colisão com moedas
	for (int i = 0; i < coinsLength; i++) {
		float coinRadius = (float)moedas.width / 4; // Ajuste o raio para metade da largura da imagem
		if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, coins[i].radius)) {
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
			// Desenhar a textura do obstáculo
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
	int heartSize = 30;  // Tamanho da imagem de coração (ajuste conforme necessário)
	int barX = 20;       // Posição X inicial
	int barY = 20;       // Posição Y inicial
	int heartSpacing = 10;  // Espaçamento entre os corações

	for (int i = 0; i < player->lives; i++) {
		// Desenhar a textura do coração para cada vida do jogador
		DrawTextureEx(vida, (Vector2) { barX + i * (heartSize + heartSpacing), barY }, 0.0f, (float)heartSize / vida.width, WHITE);
	}
}


int main(void) {
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib - Game with Game Over Screen");

	backgroundgameplayy = LoadTexture("resources/backgroundgameplayy.png");
	float scale = (float)screenWidth / backgroundgameplayy.width;

	moedas = LoadTexture("resources/coin.png");

	concreto = LoadTexture("resources/concreto.png");

	plataforma = LoadTexture("resources/plataforma.png");

	obstaculo = LoadTexture("resources/obstaculo.png");

	playerparado = LoadTexture("resources/playerparado.png");
	playerpulando = LoadTexture("resources/playerpulando.png");
	playerdireita[0] = LoadTexture("resources/playerdireta1.png");
	playerdireita[1] = LoadTexture("resources/playerdireta2.png");
	playerdireita[2] = LoadTexture("resources/playerdireta3.png");
	playeresquerda[0] = LoadTexture("resources/playeresquerda1.png");
	playeresquerda[1] = LoadTexture("resources/playeresquerda2.png");
	playeresquerda[2] = LoadTexture("resources/playeresquerda3.png");

	vida = LoadTexture("resources/vida.png");

	GameState currentState = MENU;

	int coinsCollected = 0;  // Contador de moedas coletadas

	

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

	// Inicialização do player
	Player player = { 0 };
	player.position = (Vector2){ 400, envItems[0].rect.y };
	player.speed = 0;
	player.canJump = true;  // Definimos inicialmente como falso
	player.lives = 3;
	player.damaged = false;
	player.isJumping = false;  // Não está pulando no início


	// Verificar se o jogador está em contato com o chão na inicialização
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		if (ei->blocking && CheckCollisionPointRec(player.position, ei->rect)) {
			player.canJump = true;  // Atualizamos o estado para true se o player estiver no chão
			player.speed = 0;
			player.position.y = ei->rect.y;
			break;
		}
	}


	float coinRadius = (moedas.width * coinScale) / 2.0f;

	coin coins[MAX_COINS] = {
		{{ 350, 180 }, false, coinRadius},
		{{ 690, 280 }, false, coinRadius},
		{{ 510, 30 }, false, coinRadius},
		{{ 300, -45 }, false, coinRadius}
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
			// Definir a escala da imagem para cobrir a tela inteira
			float scaleX = (float)screenWidth / backgroundgameplayy.width * 1.2f; // Aumente para 1.2f
			float scaleY = (float)screenHeight / backgroundgameplayy.height * 1.2f; // Aumente para 1.2f


			// Ajuste para o efeito de paralaxe
			
			float parallaxX = -camera.target.x * 0.1f;
			float parallaxY = -camera.target.y * 0.05f - 30; // Ajuste maior para cima


			// Desenhar a imagem de fundo com o efeito de paralaxe
			DrawTextureEx(backgroundgameplayy, (Vector2) { parallaxX, parallaxY }, 0.0f, scaleX > scaleY ? scaleX : scaleY, WHITE);


			BeginMode2D(camera);

			for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

			for (int i = 0; i < envItemsLength; i++) {
				if (i == 0) {
					// Desenhar a plataforma inicial com a textura de concreto
					DrawTexturePro(
						concreto,
						(Rectangle) {
						0, 0, concreto.width, concreto.height
					}, // Fonte (imagem completa)
						(Rectangle) {
						envItems[i].rect.x, envItems[i].rect.y, envItems[i].rect.width, envItems[i].rect.height
					}, // Destino (plataforma)
						(Vector2) {
						0, 0
					}, // Origem (sem offset)
						0.0f,               // Rotação
						WHITE               // Cor (sem modificador)
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
			}

			for (int i = 0; i < MAX_COINS; i++) {
				if (!coins[i].collected) {
					Vector2 drawPosition = { coins[i].position.x - (moedas.width * coinScale) / 2, coins[i].position.y - (moedas.height * coinScale) / 2 };
					DrawTextureEx(moedas, drawPosition, 0.0f, coinScale, WHITE);
				}
			}


			// Desenhar o sprite correto baseado no estado do player
			if (player.isJumping) {
				// Exibir o asset de pulo independentemente do movimento horizontal
				DrawTextureEx(playerpulando, (Vector2) { player.position.x - (playerpulando.width * PLAYER_SCALE) / 2, player.position.y - (playerpulando.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_LEFT)) {
				// Player andando para a esquerda quando não está pulando
				DrawTextureEx(playeresquerda[frameIndex], (Vector2) { player.position.x - (playeresquerda[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playeresquerda[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_RIGHT)) {
				// Player andando para a direita quando não está pulando
				DrawTextureEx(playerdireita[frameIndex], (Vector2) { player.position.x - (playerdireita[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playerdireita[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else {
				// Player parado no chão
				DrawTextureEx(playerparado, (Vector2) { player.position.x - (playerparado.width * PLAYER_SCALE) / 2, player.position.y - (playerparado.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}


			DrawObstacles(obstacleList);

			EndMode2D();

			DrawHealthBar(&player, screenWidth);

			// Desenha o contador de moedas na tela
			DrawText(TextFormat("Moedas coletadas: %d", coinsCollected), 20, 50, 20, BLACK);
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
	UnloadTexture(moedas);
	UnloadTexture(concreto);
	UnloadTexture(plataforma);
	UnloadTexture(obstaculo);
	UnloadTexture(playerparado);
	UnloadTexture(playerpulando);
	for (int i = 0; i < 3; i++) {
		UnloadTexture(playerdireita[i]);
		UnloadTexture(playeresquerda[i]);
	}
	UnloadTexture(vida);
	CloseWindow();
	return 0;
}