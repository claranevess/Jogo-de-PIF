#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>  // Para malloc e free
#include <stdio.h> // arquivo de texto
#include <string.h>  // Para strcpy()

// Constantes para o jogo
#define G 740                         // Gravidade aplicada ao jogador e obstáculos
#define PLAYER_JUMP_SPD 400.0f        // Velocidade de pulo do jogador
#define PLAYER_HOR_SPD 200.0f         // Velocidade horizontal do jogador
#define MAX_COINS 6                   // Número máximo de moedas no jogo
#define OBSTACLE_SPAWN_TIME 1.0f      // Intervalo de tempo para surgimento de novos obstáculos
#define OBSTACLE_HORIZONTAL_SPD 100.0f // Velocidade horizontal dos obstáculos
#define PLAYER_SCALE 0.4f             // Escala para o tamanho do sprite do jogador

// Variáveis globais para controle de contagem e estados
int leftCount = 0;           // Contador de obstáculos movendo-se para a esquerda
int leftCount2 = 0;          // Contador adicional para outro tipo de obstáculo movendo-se para a esquerda
int rightCount = 0;          // Contador de obstáculos movendo-se para a direita
int rightCount2 = 0;         // Contador adicional para outro tipo de obstáculo movendo-se para a direita

char playerName[20] = "\0";  // Nome do jogador (inicialmente vazio)
int nameIndex = 0;           // Índice para controle da entrada de texto do nome
bool nameEntered = false;    // Flag para saber se o nome do jogador foi inserido
char key;                    // Variável para capturar a entrada de caracteres
bool save = false;           // Flag para controle de salvamento do registro do jogador

int currentMap = 1;          // Variável para controlar o mapa atual
float coinScale = 0.1f;      // Escala para o tamanho das moedas

int frameIndex = 0;          // Índice para controle da animação de corrida do jogador
float frameTime = 0.0f;      // Tempo acumulado para troca de frames da animação
float frameSpeed = 0.1f;     // Velocidade de troca dos frames da animação
bool facingRight = true;     // Flag para indicar se o jogador está olhando para a direita

// Texturas usadas no jogo
Texture2D backgroundgameplayy; // Textura para o fundo do jogo
Texture2D moedas;              // Textura para as moedas
Texture2D concreto;            // Textura para o chão de concreto
Texture2D plataforma;          // Textura para as plataformas
Texture2D obstaculo;           // Textura para os obstáculos
Texture2D playerparado;        // Textura do jogador parado
Texture2D playerpulando;       // Textura do jogador pulando
Texture2D playerdireita[3];    // Array de texturas para animação do jogador correndo para a direita
Texture2D playeresquerda[3];   // Array de texturas para animação do jogador correndo para a esquerda
Texture2D vida;                // Textura para o indicador de vida do jogador

// Estrutura para representar o jogador
typedef struct Player {
	Vector2 position;         // Posição do jogador
	float speed;              // Velocidade vertical (para gravidade e pulo)
	bool canJump;             // Indica se o jogador pode pular
	int lives;                // Número de vidas do jogador
	bool damaged;             // Indica se o jogador está danificado (colidiu com um obstáculo)
	bool isJumping;           // Indica se o jogador está pulando
} Player;

// Estrutura para representar os itens do ambiente (plataformas)
typedef struct EnvItem {
	Rectangle rect;           // Retângulo que define posição e tamanho do item
	int blocking;             // Indica se o item bloqueia o jogador (1 = sim, 0 = não)
	Color color;              // Cor do item (usado para desenhar)
} EnvItem;

// Estrutura para representar uma moeda
typedef struct coin {
	Vector2 position;         // Posição da moeda
	bool collected;           // Indica se a moeda foi coletada
	float radius;             // Raio da moeda (para colisão)
} coin;

// Estrutura para representar um obstáculo
typedef struct Obstacle {
	Vector2 position;         // Posição do obstáculo
	Vector2 speed;            // Velocidade do obstáculo
	bool active;              // Indica se o obstáculo está ativo
	bool movingLeft;          // Indica se o obstáculo está se movendo para a esquerda
	Color color;              // Cor do obstáculo (usado para desenhar)
} Obstacle;

// Estrutura para um nó da lista encadeada de obstáculos
typedef struct ObstacleNode {
	Obstacle obstacle;        // Obstáculo armazenado no nó
	struct ObstacleNode* next; // Ponteiro para o próximo nó
} ObstacleNode;

// Estrutura para armazenar o registro de um jogador
typedef struct {
	char name[20];            // Nome do jogador (máximo de 20 caracteres)
	int coinsCollected;       // Número de moedas coletadas pelo jogador
} PlayerRecord;

// Enumeração para definir os diferentes estados do jogo
typedef enum {
	MENU,                     // Estado do menu principal
	GAMEPLAY,                 // Estado de gameplay (jogo em andamento)
	GAMEOVER,                 // Estado de game over (fim de jogo)
	LEADERBOARD               // Estado de leaderboard (placar de melhores jogadores)
} GameState;

// Declaração das funções principais do jogo
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected);
// Atualiza o estado do jogador (movimentação, colisões, coleta de moedas)

void AddObstacle(ObstacleNode** head, Vector2 spawnPosition);
// Adiciona um obstáculo que se move para a esquerda ou para a direita à lista de obstáculos

void AddObstacle2(ObstacleNode** head, Vector2 spawnPosition);
// Adiciona um segundo tipo de obstáculo com comportamento diferente à lista

void UpdateObstacles(ObstacleNode* head, EnvItem* envItems, int envItemsLength, float delta, Player* player, GameState* currentState);
// Atualiza a posição e o estado dos obstáculos (movimentação e colisões)

void RemoveInactiveObstacles(ObstacleNode** head);
// Remove obstáculos que estão fora da tela ou que não estão mais ativos

void DrawObstacles(ObstacleNode* head);
// Desenha os obstáculos ativos na tela

void FreeObstacleList(ObstacleNode* head);
// Libera a memória alocada para a lista de obstáculos

void DrawHealthBar(Player* player, int screenWidth);
// Desenha a barra de vida do jogador na tela

// Declarações das funções para manipulação de arquivo
void SavePlayerRecord(const char* filename, PlayerRecord* player);
// Salva o registro do jogador (nome e número de moedas coletadas) em um arquivo

int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);
// Carrega registros de jogadores de um arquivo e retorna o número de registros carregados

// Função para comparar dois registros de jogadores
int ComparePlayerRecords(const void* a, const void* b) {
	PlayerRecord* recordA = (PlayerRecord*)a;
	PlayerRecord* recordB = (PlayerRecord*)b;
	// Ordena em ordem decrescente com base nas moedas coletadas
	return recordB->coinsCollected - recordA->coinsCollected;
}

// Função para salvar o registro do jogador no arquivo
void SavePlayerRecord(const char* filename, PlayerRecord* player) {
	// Carregar registros existentes do arquivo para a memória
	PlayerRecord records[100];  // Array para armazenar até 100 registros
	int recordCount = LoadPlayerRecords(filename, records, 100);

	// Adicionar o novo registro à lista
	records[recordCount] = *player;
	recordCount++;

	// Ordenar a lista de registros em ordem decrescente de moedas coletadas
	qsort(records, recordCount, sizeof(PlayerRecord), ComparePlayerRecords);

	// Abrir o arquivo para escrita (sobrescrever o arquivo existente)
	FILE* file = fopen(filename, "w");

	if (file == NULL) {
		printf("Erro ao abrir o arquivo para escrita.\n");
		return;
	}

	// Escrever todos os registros ordenados no arquivo
	for (int i = 0; i < recordCount; i++) {
		fprintf(file, "%s %d\n", records[i].name, records[i].coinsCollected);
	}

	// Fechar o arquivo após a escrita
	fclose(file);
}

// Função para carregar registros de jogadores de um arquivo
int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords) {
	// Abrir o arquivo para leitura
	FILE* file = fopen(filename, "r");

	if (file == NULL) {
		printf("Erro ao abrir o arquivo para leitura.\n");
		return 0;  // Retorna 0 se não conseguiu abrir o arquivo
	}

	int count = 0;  // Contador para o número de registros carregados

	// Ler o nome e o número de moedas coletadas de cada registro
	// Limita a leitura a 19 caracteres para o nome (devido ao tamanho máximo do array)
	while (count < maxRecords && fscanf(file, "%19s %d", records[count].name, &records[count].coinsCollected) == 2) {
		count++;  // Incrementa o contador se a leitura foi bem-sucedida
	}

	// Fechar o arquivo após a leitura
	fclose(file);
	return count;  // Retorna o número de registros carregados
}

// Função para exibir os melhores jogadores (leaderboard)
void DisplayTopPlayers(PlayerRecord* records, int count, int screenWidth) {
	// Verifica se há registros para exibir
	if (count == 0) {
		// Exibe uma mensagem se não houver registros
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

// Declarações das funções de manipulação de arquivo
void SavePlayerRecord(const char* filename, PlayerRecord* player);
// Salva o registro do jogador no arquivo

int LoadPlayerRecords(const char* filename, PlayerRecord* records, int maxRecords);
// Carrega registros de jogadores a partir de um arquivo

// Função para transição entre mapas (fases)
void TransitionMap(GameState* currentState, int* currentMap, int nextMap) {
	float fadeOutDuration = 1.0f;  // Duração do efeito de fade out
	float fadeInDuration = 1.0f;   // Duração do efeito de fade in
	float timer = 0.0f;

	// Iniciar o efeito de fade out
	while (timer < fadeOutDuration) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Mudando de mapa...", 360, 220, 20, WHITE);
		EndDrawing();
		timer += GetFrameTime();
	}

	*currentMap = nextMap;  // Atualiza para o próximo mapa

	// Resetar o timer e iniciar o efeito de fade in
	timer = 0.0f;
	while (timer < fadeInDuration) {
		float alpha = (1.0f - (timer / fadeInDuration)) * 255;
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Carregando mapa...", 360, 220, 20, Fade(WHITE, alpha));
		EndDrawing();
		timer += GetFrameTime();
	}

	*currentState = GAMEPLAY;  // Retorna ao estado de gameplay
}

// Função para atualizar o estado do jogador
void UpdatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float delta, coin* coins, int coinsLength, GameState* currentState, int* coinsCollected) {
	float playerAltura = 40.0f;  // Altura aproximada do jogador

	// Movimentação para a esquerda
	if (IsKeyDown(KEY_LEFT)) {
		player->position.x -= PLAYER_HOR_SPD * delta;
		facingRight = false;

		// Atualizar a animação de corrida
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3;  // Cicla entre os frames
			frameTime = 0.0f;
		}
	}
	// Movimentação para a direita
	else if (IsKeyDown(KEY_RIGHT)) {
		player->position.x += PLAYER_HOR_SPD * delta;
		facingRight = true;

		// Atualizar a animação de corrida
		frameTime += delta;
		if (frameTime >= frameSpeed) {
			frameIndex = (frameIndex + 1) % 3;  // Cicla entre os frames
			frameTime = 0.0f;
		}
	}
	else {
		// Resetar a animação quando o jogador está parado
		frameIndex = 0;
	}

	// Lógica para o pulo do jogador
	if (IsKeyPressed(KEY_SPACE) && player->canJump) {
		player->speed = -PLAYER_JUMP_SPD;  // Impulso para o pulo
		player->canJump = false;           // Não permite pular novamente no ar
		player->isJumping = true;          // Define o estado como pulando
	}

	// Aplicar gravidade
	player->speed += G * delta;
	player->position.y += player->speed * delta;

	// Verificar colisão com o chão
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		if (ei->blocking && CheckCollisionPointRec((Vector2) { player->position.x, player->position.y + playerAltura }, ei->rect)) {
			player->canJump = true;
			player->isJumping = false;
			player->speed = 0;

			// Ajustar a posição do jogador para ficar na borda superior da plataforma
			player->position.y = ei->rect.y - playerAltura + 1;
			if (ei->rect.y == -130) {
				TransitionMap(currentState, &currentMap, currentMap + 1);
			}
			break;
		}
	}

	// Verificar se o jogador caiu fora da tela
	if (player->position.y > 600) {
		player->lives--;
		if (player->lives <= 0) {
			*currentState = GAMEOVER;
		}
		else {
			// Reiniciar a posição do jogador
			player->position = (Vector2){ 400, 280 };
			player->speed = 0;
		}
	}

	// Verificar colisão com moedas
	for (int i = 0; i < coinsLength; i++) {
		float coinRadius = (float)moedas.width / 4;
		if (!coins[i].collected && CheckCollisionCircles(player->position, 20, coins[i].position, coins[i].radius)) {
			coins[i].collected = true;
			(*coinsCollected)++;
		}
	}
}

// Função para adicionar um obstáculo à lista
void AddObstacle(ObstacleNode** head, Vector2 spawnPosition) {
	ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
	if (newNode == NULL) return;

	// Inicializar o novo obstáculo
	newNode->obstacle.position = spawnPosition;
	newNode->obstacle.speed = (Vector2){ 0.0f, 0.0f };
	newNode->obstacle.active = true;
	newNode->obstacle.color = RED;

	// Determinar a direção do obstáculo (esquerda ou direita)
	if (leftCount <= rightCount) {
		newNode->obstacle.movingLeft = true;
		leftCount++;
	}
	else {
		newNode->obstacle.movingLeft = false;
		rightCount++;
	}

	// Inserir o novo obstáculo no início da lista
	newNode->next = *head;
	*head = newNode;
}
// Função alternativa para adicionar outro tipo de obstáculo
void AddObstacle2(ObstacleNode** head, Vector2 spawnPosition) {
	// Aloca memória para um novo nó de obstáculo
	ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
	// Verifica se a alocação de memória falhou
	if (newNode == NULL) return;

	// Define a posição inicial do obstáculo
	newNode->obstacle.position = spawnPosition;
	// Define a velocidade inicial como zero
	newNode->obstacle.speed = (Vector2){ 0.0f, 0.0f };
	// Marca o obstáculo como ativo
	newNode->obstacle.active = true;
	// Define a cor do obstáculo como vermelho
	newNode->obstacle.color = RED;

	// Determina a direção do obstáculo com base nos contadores
	if (leftCount2 <= rightCount2) {
		// Move o obstáculo para a esquerda se houver menos obstáculos à esquerda
		newNode->obstacle.movingLeft = true;
		leftCount2++;
	}
	else {
		// Move o obstáculo para a direita
		newNode->obstacle.movingLeft = false;
		rightCount2++;
	}

	// Insere o novo obstáculo no início da lista
	newNode->next = *head;
	*head = newNode;
}

// Função de atualização de obstáculos
void UpdateObstacles(ObstacleNode* head, EnvItem* envItems, int envItemsLength, float delta, Player* player, GameState* currentState) {
	ObstacleNode* current = head;  // Aponta para o início da lista de obstáculos
	bool playerHit = false;        // Flag para verificar se o jogador foi atingido

	// Percorre a lista de obstáculos
	while (current != NULL) {
		// Verifica se o obstáculo está inativo
		if (!current->obstacle.active) {
			current = current->next;
			continue;  // Pula para o próximo obstáculo
		}

		bool hitObstacle = false;  // Flag para colisão com o ambiente

		// Aplica a gravidade ao obstáculo
		current->obstacle.speed.y += G * delta;
		// Atualiza a posição vertical do obstáculo
		current->obstacle.position.y += current->obstacle.speed.y * delta;

		// Movimenta o obstáculo para a esquerda ou direita
		if (current->obstacle.movingLeft) {
			current->obstacle.position.x -= OBSTACLE_HORIZONTAL_SPD * delta;
		}
		else {
			current->obstacle.position.x += OBSTACLE_HORIZONTAL_SPD * delta;
		}

		// Verifica colisão com itens do ambiente
		for (int j = 0; j < envItemsLength; j++) {
			EnvItem* ei = &envItems[j];  // Aponta para o item atual do ambiente
			// Verifica colisão com o obstáculo
			if (ei->blocking &&
				current->obstacle.position.x + 10 >= ei->rect.x &&
				current->obstacle.position.x - 10 <= ei->rect.x + ei->rect.width &&
				current->obstacle.position.y + 20 >= ei->rect.y &&
				current->obstacle.position.y <= ei->rect.y + current->obstacle.speed.y * delta) {
				hitObstacle = true;  // Colisão detectada
				// Faz o obstáculo "quicar"
				current->obstacle.speed.y = -PLAYER_JUMP_SPD / 2;
				// Ajusta a posição para ficar acima do item
				current->obstacle.position.y = ei->rect.y - 20;
				break;
			}
		}

		// Desativa o obstáculo se ele sair da tela
		if (current->obstacle.position.x > 850 || current->obstacle.position.x < -50) {
			current->obstacle.active = false;
		}

		// Desativa o obstáculo se ele cair fora da tela
		if (!hitObstacle && current->obstacle.position.y > 600) {
			current->obstacle.active = false;
		}

		// Verifica colisão com o jogador
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
			current->obstacle.active = false;  // Marcar o obstáculo como inativo
			playerHit = true;
		}


		// Move para o próximo obstáculo na lista
		current = current->next;
	}

	// Reseta o estado de dano do jogador se ele não foi atingido
	if (!playerHit) {
		player->damaged = false;
	}
}

// Função para remover obstáculos inativos
void RemoveInactiveObstacles(ObstacleNode** head) {
	ObstacleNode* current = *head;  // Aponta para o início da lista
	ObstacleNode* prev = NULL;      // Ponteiro para o nó anterior

	// Percorre a lista de obstáculos
	while (current != NULL) {
		// Verifica se o obstáculo está inativo
		if (!current->obstacle.active) {
			// Remove o obstáculo da lista
			if (prev == NULL) {
				*head = current->next;
			}
			else {
				prev->next = current->next;
			}
			// Libera a memória do obstáculo
			ObstacleNode* temp = current;
			current = current->next;
			free(temp);
		}
		else {
			prev = current;  // Atualiza o nó anterior
			current = current->next;
		}
	}
}

// Função para desenhar obstáculos
void DrawObstacles(ObstacleNode* head) {
	ObstacleNode* current = head;  // Aponta para o início da lista
	// Percorre a lista de obstáculos
	while (current != NULL) {
		// Desenha o obstáculo se estiver ativo
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
		current = current->next;  // Move para o próximo obstáculo
	}
}

// Função para liberar memória da lista de obstáculos
void FreeObstacleList(ObstacleNode* head) {
	ObstacleNode* current = head;
	// Percorre a lista e libera a memória de cada nó
	while (current != NULL) {
		ObstacleNode* temp = current;
		current = current->next;
		free(temp);
	}
}

// Função para desenhar a barra de vidas do jogador
void DrawHealthBar(Player* player, int screenWidth) {
	int heartSize = 30;       // Define o tamanho da imagem de coração
	int barX = 20;            // Posição X inicial
	int barY = 20;            // Posição Y inicial
	int heartSpacing = 10;    // Espaçamento entre os corações

	// Desenha um coração para cada vida do jogador
	for (int i = 0; i < player->lives; i++) {
		DrawTextureEx(vida, (Vector2) { barX + i * (heartSize + heartSpacing), barY }, 0.0f, (float)heartSize / vida.width, WHITE);
	}
}
// Função principal do programa
int main(void) {
	const int screenWidth = 800;  // Largura da janela
	const int screenHeight = 450; // Altura da janela

	// Inicializa a janela do jogo
	InitWindow(screenWidth, screenHeight, "raylib - Game with Game Over Screen");

	// Carregamento das texturas
	backgroundgameplayy = LoadTexture("resources/backgroundgameplayy.png");
	float scale = (float)screenWidth / backgroundgameplayy.width;  // Escala do fundo

	moedas = LoadTexture("resources/coin.png");
	concreto = LoadTexture("resources/concreto.png");
	plataforma = LoadTexture("resources/plataforma.png");
	obstaculo = LoadTexture("resources/obstaculo.png");

	// Carregamento das texturas do jogador
	playerparado = LoadTexture("resources/playerparado.png");
	playerpulando = LoadTexture("resources/playerpulando.png");
	playerdireita[0] = LoadTexture("resources/playerdireta1.png");
	playerdireita[1] = LoadTexture("resources/playerdireta2.png");
	playerdireita[2] = LoadTexture("resources/playerdireta3.png");
	playeresquerda[0] = LoadTexture("resources/playeresquerda1.png");
	playeresquerda[1] = LoadTexture("resources/playeresquerda2.png");
	playeresquerda[2] = LoadTexture("resources/playeresquerda3.png");

	vida = LoadTexture("resources/vida.png");  // Textura da vida do jogador

	GameState currentState = MENU;  // Define o estado inicial do jogo como MENU

	int coinsCollected = 0;  // Contador de moedas coletadas

	// Definição dos itens do ambiente
	EnvItem envItems[] = {
		{{ -300, 400, 1300, 300 }, 1, GRAY },
		{{ 300, 200, 400, 10 }, 1, GRAY },
		{{ 250, 300, 100, 10 }, 1, GRAY },
		{{ 650, 300, 100, 10 }, 1, GRAY },
		{{ 250, 100, 100, 10 }, 1, GRAY },
		{{ 650, 100, 100, 10 }, 1, GRAY },
		{{ 450 - 37.5f, 50, 180, 10 }, 1, GRAY },
		{{ 220, -30, 100, 10 }, 1, GRAY },
		{{ -130, -130, 300, 10 }, 1, GRAY }
	};
	int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);  // Calcula o tamanho do array

	// Inicialização do jogador
	Player player = { 0 };
	player.position = (Vector2){ 400, envItems[0].rect.y };
	player.speed = 0;
	player.canJump = true;    // Define que o jogador pode pular inicialmente
	player.lives = 3;         // Define o número de vidas
	player.damaged = false;   // O jogador não está danificado inicialmente
	player.isJumping = false; // O jogador não está pulando inicialmente

	// Verifica se o jogador está em contato com o chão na inicialização
	for (int i = 0; i < envItemsLength; i++) {
		EnvItem* ei = &envItems[i];
		// Se o jogador colidir com um item bloqueante, permite o pulo
		if (ei->blocking && CheckCollisionPointRec(player.position, ei->rect)) {
			player.canJump = true;
			player.speed = 0;
			player.position.y = ei->rect.y;
			break;
		}
	}

	// Definição do raio da moeda para colisão
	float coinRadius = (moedas.width * coinScale) / 2.0f;

	// Inicialização das moedas
	coin coins[MAX_COINS] = {
		{{ 350, 180 }, false, coinRadius },
		{{ 690, 280 }, false, coinRadius },
		{{ 510, 30 }, false, coinRadius },
		{{ 300, -45 }, false, coinRadius }
	};

	ObstacleNode* obstacleList = NULL;   // Ponteiro para a lista de obstáculos
	float obstacleSpawnTimer = 0.0f;     // Timer para controlar a geração de obstáculos

	// Configuração da câmera
	Camera2D camera = { 0 };
	camera.target = player.position;
	camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	SetTargetFPS(60);  // Define o FPS do jogo para 60

	// Loop principal do jogo
	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime();  // Calcula o tempo entre frames

		// Lógica para o estado MENU
		if (currentState == MENU) {
			if (IsKeyPressed(KEY_ENTER)) {
				currentState = GAMEPLAY;  // Inicia o jogo ao pressionar ENTER
			}
		}
		// Lógica para o estado GAMEPLAY
		else if (currentState == GAMEPLAY) {
			// Atualiza o estado do jogador
			UpdatePlayer(&player, envItems, envItemsLength, deltaTime, coins, MAX_COINS, &currentState, &coinsCollected);

			// Incrementa o timer e gera novos obstáculos quando necessário
			obstacleSpawnTimer += deltaTime;
			if (obstacleSpawnTimer >= OBSTACLE_SPAWN_TIME) {
				obstacleSpawnTimer = 0.0f;
				AddObstacle(&obstacleList, (Vector2) { 500, 100 });
				AddObstacle2(&obstacleList, (Vector2) { 500, -100 });
			}

			// Atualiza os obstáculos e remove os inativos
			UpdateObstacles(obstacleList, envItems, envItemsLength, deltaTime, &player, &currentState);
			RemoveInactiveObstacles(&obstacleList);

			// Ajusta a câmera para seguir o jogador
			camera.target = player.position;

			// Ajusta o zoom da câmera com a rolagem do mouse
			camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
			if (camera.zoom > 3.0f) camera.zoom = 3.0f;
			else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

			// Reseta o zoom e a posição do jogador ao pressionar R
			if (IsKeyPressed(KEY_R)) {
				camera.zoom = 1.0f;
				player.position = (Vector2){ 400, 280 };
			}
		}

		BeginDrawing();  // Inicia o processo de desenho na tela
		ClearBackground(RAYWHITE);  // Limpa a tela com a cor de fundo branco

		// Verifica se o estado atual do jogo é MENU
		if (currentState == MENU) {
			ClearBackground(SKYBLUE);  // Define o fundo como azul claro no menu
			DrawText("Bem-vindo ao Plataformia!", screenWidth / 2 - MeasureText("Bem-vindo ao Plataformia!", 20) / 2, screenHeight / 2 - 20, 20, BLACK);  // Mensagem de boas-vindas centralizada
			DrawText("Aperte ENTER para começar", screenWidth / 2 - MeasureText("Aperte ENTER para começar", 20) / 2, screenHeight / 2 + 10, 20, DARKGRAY);  // Instrução para iniciar o jogo
			DrawText("Use as setas do teclado para se mover e a barra de espaço para pular", screenWidth / 2 - MeasureText("Use as setas do teclado para se mover e a barra de espaço para pular", 20) / 2, screenHeight / 2 + 40, 20, DARKGRAY);  // Instruções de controle
		}

		// Verifica se o estado atual do jogo é GAMEPLAY
		else if (currentState == GAMEPLAY) {
			// Define a escala da imagem de fundo para cobrir a tela inteira
			float scaleX = (float)screenWidth / backgroundgameplayy.width * 1.2f;
			float scaleY = (float)screenHeight / backgroundgameplayy.height * 1.2f;

			// Calcula o efeito de paralaxe para o fundo
			float parallaxX = -camera.target.x * 0.1f;
			float parallaxY = -camera.target.y * 0.05f - 30;  // Move o fundo um pouco para cima

			// Desenha a imagem de fundo com o efeito de paralaxe
			DrawTextureEx(backgroundgameplayy, (Vector2) { parallaxX, parallaxY }, 0.0f, scaleX > scaleY ? scaleX : scaleY, WHITE);

			BeginMode2D(camera);  // Inicia o modo 2D, aplicando a posição da câmera

			// Desenha cada item do ambiente como um retângulo
			for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

			// Desenha as plataformas com texturas específicas
			for (int i = 0; i < envItemsLength; i++) {
				if (i == 0) {
					// Desenha a primeira plataforma com a textura de concreto
					DrawTexturePro(
						concreto,
						(Rectangle) {
						0, 0, concreto.width, concreto.height
					},  // Fonte (imagem completa)
						(Rectangle) {
						envItems[i].rect.x, envItems[i].rect.y, envItems[i].rect.width, envItems[i].rect.height
					},  // Destino (plataforma)
						(Vector2) {
						0, 0
					},  // Origem (sem offset)
						0.0f,               // Rotação
						WHITE               // Cor (sem modificador)
					);
				}
				else {
					// Desenha as outras plataformas com a textura padrão
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

			// Desenha todas as moedas que ainda não foram coletadas
			for (int i = 0; i < MAX_COINS; i++) {
				if (!coins[i].collected) {
					Vector2 drawPosition = { coins[i].position.x - (moedas.width * coinScale) / 2, coins[i].position.y - (moedas.height * coinScale) / 2 };
					DrawTextureEx(moedas, drawPosition, 0.0f, coinScale, WHITE);  // Desenha a moeda com a escala definida
				}
			}

			// Desenha o sprite correto do jogador, dependendo do seu estado (pulo ou movimentação)
			if (player.isJumping) {
				// Desenha o sprite de pulo do jogador
				DrawTextureEx(playerpulando, (Vector2) { player.position.x - (playerpulando.width * PLAYER_SCALE) / 2, player.position.y - (playerpulando.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_LEFT)) {
				// Desenha o sprite do jogador andando para a esquerda
				DrawTextureEx(playeresquerda[frameIndex], (Vector2) { player.position.x - (playeresquerda[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playeresquerda[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else if (IsKeyDown(KEY_RIGHT)) {
				// Desenha o sprite do jogador andando para a direita
				DrawTextureEx(playerdireita[frameIndex], (Vector2) { player.position.x - (playerdireita[frameIndex].width * PLAYER_SCALE) / 2, player.position.y - (playerdireita[frameIndex].height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}
			else {
				// Desenha o sprite do jogador parado
				DrawTextureEx(playerparado, (Vector2) { player.position.x - (playerparado.width * PLAYER_SCALE) / 2, player.position.y - (playerparado.height * PLAYER_SCALE) / 2 }, 0.0f, PLAYER_SCALE, WHITE);
			}

			// Desenha todos os obstáculos ativos na tela
			DrawObstacles(obstacleList);

			EndMode2D();  // Termina o modo 2D

			// Desenha a barra de vida do jogador
			DrawHealthBar(&player, screenWidth);

			// Exibe o contador de moedas coletadas no canto superior esquerdo
			DrawText(TextFormat("Moedas coletadas: %d", coinsCollected), 20, 50, 20, BLACK);
		}

		// Verifica se o estado atual do jogo é LEADERBOARD
		else if (currentState == LEADERBOARD) {
			ClearBackground(SKYBLUE);  // Define o fundo como azul claro

			// Carrega e exibe os registros dos melhores jogadores
			PlayerRecord records[100];
			int recordCount = LoadPlayerRecords("records.txt", records, 100);
			DisplayTopPlayers(records, recordCount, screenWidth);  // Exibe os 5 melhores jogadores

			// Verifica se o jogador pressiona ENTER para voltar ao menu
			if (IsKeyPressed(KEY_ENTER)) {
				currentState = MENU;
				player.position = (Vector2){ 400, 280 };  // Reseta a posição do jogador
				player.speed = 0;
				player.lives = 3;
				coinsCollected = 0;
			}
		}

		// Verifica se o estado atual do jogo é GAMEOVER
		else if (currentState == GAMEOVER) {
			ClearBackground(RED);  // Define o fundo como vermelho
			DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 60, 40, BLACK);  // Exibe a mensagem de Game Over
			DrawText("Digite seu nome e pressione ENTER", screenWidth / 2 - 200, screenHeight / 2, 20, DARKGRAY);  // Instrução para o jogador
			DrawText(playerName, screenWidth / 2 - 100, screenHeight / 2 + 40, 20, BLACK);  // Exibe o nome do jogador digitado até o momento

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