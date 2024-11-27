#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>
#include <pthread.h>
#include "jogo_dino.h"
#include "missel.h"

#define MAX_DINOS 5
#define ALTURA_JANELA 40
#define LARGURA_JANELA 120

const int max_vida_dino_por_dificuldade[3] = {2, 3, 4};
const int max_misseis_por_dificuldade[3] = {15, 10, 8};
const int tempo_novo_dino_por_dificuldade[3] = {15, 10, 5};

int max_vida_dino;
int max_misseis;
int tempo_novo_dino;
int dinos_vivos;
bool fim_jogo = false;
bool vitoria = false;

pthread_mutex_t mutex_deposito = PTHREAD_MUTEX_INITIALIZER;

janela_jogo_t janela_jogo;
helicoptero_t helicoptero;
deposito_t deposito;
caminhao_t caminhao;
dino_t dinos[MAX_DINOS];
missel_t* lista_missel;

void obter_dificuldade() {
  int dificuldade;

  while (true) {
    printf("1 - Fácil\n");
    printf("2 - Médio\n");
    printf("3 - Difícil\n");
    printf("Escolha um nível de dificuldade: ");
    scanf("%d", &dificuldade);
    if (dificuldade > 0 &&  dificuldade < 4) {
      break;
    }
    printf("Informe uma dificuldade válida!\n");
  }

  dificuldade--;
  max_vida_dino = max_vida_dino_por_dificuldade[dificuldade];
  max_misseis = max_misseis_por_dificuldade[dificuldade];
  tempo_novo_dino = tempo_novo_dino_por_dificuldade[dificuldade];
}

void verifica_colisao() {
  int i;
  for (i = 0; i < MAX_DINOS; i++) {
    if (!dinos[i].ativo) continue;
    if ((helicoptero.y == dinos[i].y || helicoptero.y == dinos[i].y + 1) && helicoptero.x == dinos[i].x) {
      fim_jogo = true;
    }
  }
}

void verifica_acerto(missel_t* missel) {
  int i;
  for (i = 0; i < MAX_DINOS; i++) {
    if (!dinos[i].ativo) continue;
    if (missel->y == dinos[i].y && missel->x == dinos[i].x) {
      missel->acertou = true;
      dinos[i].vida--;
      if (!dinos[i].vida) {
        dinos[i].ativo = false;
        dinos_vivos--;
      }
    }
  }
}

void* mover_tiro(void* arg) {
  missel_t* missel = (missel_t*) arg;
  while (!missel->acertou && missel->x < LARGURA_JANELA - 1) {
    missel->x++;
    verifica_acerto(missel);
    usleep(30000);
  }
  remover_missel_lista(missel, &lista_missel);
  return NULL;
}

void atirar() {
  if (helicoptero.misseis <= 0) return;
  helicoptero.misseis--;
  missel_t* missel = (missel_t*) malloc(sizeof(missel_t));
  missel->id = rand();
  missel->x = helicoptero.x;
  missel->y = helicoptero.y;
  adicionar_missel_lista(missel, &lista_missel);
  pthread_create(&missel->thread, NULL, mover_tiro, missel);
}

void* mover_helicoptero(void* arg) {
  while (!fim_jogo) {
    int ch = getch();
    switch (ch) {
      case 'w': helicoptero.y--; break;
      case 's': helicoptero.y++; break;
      case 'a': helicoptero.x--; break;
      case 'd': helicoptero.x++; break;
      case ' ': atirar(); break;
      case 'q': fim_jogo = true; break;
    }

    if (helicoptero.y < 1) helicoptero.y = 1;
    if (helicoptero.y >= ALTURA_JANELA - 2) helicoptero.y = ALTURA_JANELA - 3;
    if (helicoptero.x < 1) helicoptero.x = 1;
    if (helicoptero.x >= LARGURA_JANELA - 1) helicoptero.x = LARGURA_JANELA - 2;
  
    verifica_colisao();
    usleep(30000);
  }
  return NULL;
}

void* recarregar_helicoptero(void* arg) {
  while (!fim_jogo) {
    pthread_mutex_lock(&mutex_deposito);
    while (
      deposito.misseis > 0
      && helicoptero.misseis < max_misseis
      && helicoptero.y == deposito.y - 1
      && helicoptero.x >= deposito.x
      && helicoptero.x < deposito.x + 4
    ) {
      helicoptero.misseis++;
      deposito.misseis--;
      usleep(600000);
    }
    pthread_mutex_unlock(&mutex_deposito);
    usleep(30000);
  }
  return NULL;
}

void recarregar_deposito() {
  while (deposito.misseis == max_misseis);
  pthread_mutex_lock(&mutex_deposito);
  while (deposito.misseis < max_misseis) {
    deposito.misseis++;
    usleep(600000);
  }
  pthread_mutex_unlock(&mutex_deposito);
}

void* mover_caminhao(void* arg) {
  while (!fim_jogo) {
    while (caminhao.indo_deposito) {
      if (caminhao.x > (deposito.x + 4)) {
        caminhao.x--;
        usleep(90000);
      } else {
        recarregar_deposito();
        caminhao.indo_deposito = false;
      }
    }
    while (!caminhao.indo_deposito) {
      if (caminhao.x < LARGURA_JANELA) {
        caminhao.x++;
        usleep(50000);
      } else {
        sleep(1);
        caminhao.indo_deposito = true;
      }
    }
  }
  return NULL;
}

void* mover_dinos(void* arg) {
  while (!fim_jogo) {
    int i;
    for (i = 0; i < MAX_DINOS; i++) {
      if (!dinos[i].ativo) continue;

      dinos[i].indo_frente ? dinos[i].x++ : dinos[i].x--;
      dinos[i].indo_cima ? dinos[i].y-- : dinos[i].y++;

      if (dinos[i].y < ALTURA_JANELA / 2) {
        dinos[i].y = ALTURA_JANELA / 2;
        dinos[i].indo_cima = false;
      }
      if (dinos[i].y >= ALTURA_JANELA - 2) {
        dinos[i].y = ALTURA_JANELA - 3;
        dinos[i].indo_cima = true;
      }
      if (dinos[i].x < 1) {
        dinos[i].x = 1;
        dinos[i].indo_frente = true;
      }
      if (dinos[i].x >= LARGURA_JANELA - 1) {
        dinos[i].x = LARGURA_JANELA - 2;
        dinos[i].indo_frente = false;
      }
    }
    usleep(250000);
  }
  return NULL;
}

void* criar_dinos(void* arg) {
  while (!fim_jogo) {
    sleep(tempo_novo_dino);
    if (!dinos[0].ativo) return NULL;
    int i;
    for (i = 1; i < MAX_DINOS; i++) {
      if (!dinos[i].ativo) {
        dinos[i].x = dinos[0].x+1;
        dinos[i].y = dinos[0].y;
        dinos[i].vida = max_vida_dino;
        dinos[i].ativo = true;
        dinos[i].indo_frente = !dinos[0].indo_frente;
        dinos[i].indo_cima = false;
        dinos_vivos++;
        break;
      }
    }
  }
  return NULL;
}

void desenhar_janela() {
  delwin(janela_jogo.win);
  janela_jogo.win = newwin(ALTURA_JANELA, LARGURA_JANELA, janela_jogo.y, janela_jogo.x);
  box(janela_jogo.win, 0, 0);
}

void desenhar_infos() {
  mvwprintw(janela_jogo.win, 0, 1, " Mísseis: %d | Depósito: %d | Dinos vivos: %d ", helicoptero.misseis, deposito.misseis, dinos_vivos);
  mvwprintw(janela_jogo.win, 0, LARGURA_JANELA - 22, " Aperte 'Q' para sair ");
  mvprintw(janela_jogo.y + ALTURA_JANELA, janela_jogo.x + 1, "'WASD' para mover e 'Espaço' para atirar");
  mvprintw(janela_jogo.y + ALTURA_JANELA + 1, janela_jogo.x + 1, "H: Helicóptero | D: Dinossauro | C: Caminhão | B: Base");
}

void desenhar_helicoptero() {
  mvwprintw(janela_jogo.win, helicoptero.y, helicoptero.x, "H");
}

void desenhar_misseis() {
  missel_t* missel = lista_missel;
  while (missel != NULL) {
    mvwprintw(janela_jogo.win, missel->y, missel->x, "-");
    missel = missel->prox;
  }
}

void desenhar_deposito() {
  mvwprintw(janela_jogo.win, deposito.y, deposito.x, "BBBB");
}

void desenhar_caminhao() {
  mvwprintw(janela_jogo.win, caminhao.y, caminhao.x, "CC");
}

void desenhar_dinos() {
  int i;
  for (i = 0; i < MAX_DINOS; i++) {
    if (dinos[i].ativo) {
      mvwprintw(janela_jogo.win, dinos[i].y - 1, dinos[i].x, "%d", dinos[i].vida);
      mvwprintw(janela_jogo.win, dinos[i].y, dinos[i].x, "D");
      mvwprintw(janela_jogo.win, dinos[i].y + 1, dinos[i].x, "D");
    }
  }
}

void desenhar_entidades() {
  desenhar_janela();
  desenhar_infos();
  desenhar_helicoptero();
  desenhar_misseis();
  desenhar_deposito();
  desenhar_caminhao();
  desenhar_dinos();
}

void inicializar_entidades() {
  janela_jogo.x = (COLS - LARGURA_JANELA) / 2;
  janela_jogo.y = (LINES - ALTURA_JANELA) / 2;
  helicoptero.x = LARGURA_JANELA / 2;
  helicoptero.y = ALTURA_JANELA / 2;
  helicoptero.misseis = max_misseis;
  deposito.x = LARGURA_JANELA / 2;
  deposito.y = ALTURA_JANELA - 2;
  deposito.misseis = 0;
  caminhao.x = LARGURA_JANELA - 2;
  caminhao.y = ALTURA_JANELA - 2;
  caminhao.indo_deposito = true;
  dinos[0].x = LARGURA_JANELA - 2;
  dinos[0].y = ALTURA_JANELA / 2;
  dinos[0].vida = max_vida_dino;
  dinos[0].ativo = true;
  dinos[0].indo_frente = false;
  dinos[0].indo_cima = false;
  dinos_vivos = 1;
}

void verificar_status_jogo() {
  if (!dinos_vivos || dinos_vivos == MAX_DINOS) {
    vitoria = !dinos_vivos;
    fim_jogo = true;
  }
}

void apresentar_mensagem_final() {
  int desloc_msg = vitoria ? 11 : 15;
  clear();
  desenhar_entidades();
  mvwprintw(janela_jogo.win, ALTURA_JANELA / 2, (LARGURA_JANELA / 2) - desloc_msg, vitoria ? "Parabéns, você venceu!" : "Que pena, não foi dessa vez...");
  mvwprintw(janela_jogo.win, ALTURA_JANELA / 2 + 1, (LARGURA_JANELA / 2) - 10, "Aperte 'Q' para sair");
  refresh();
  wrefresh(janela_jogo.win);
  nodelay(stdscr, FALSE);
  while (getch() != 'q');
}

void jogo() {
  inicializar_entidades();

  pthread_t th_helicoptero, th_recarga_helicoptero, th_caminhao, th_dinos, th_cria_dinos;

  pthread_create(&th_helicoptero, NULL, mover_helicoptero, NULL);
  pthread_create(&th_recarga_helicoptero, NULL, recarregar_helicoptero, NULL);
  pthread_create(&th_caminhao, NULL, mover_caminhao, NULL);
  pthread_create(&th_dinos, NULL, mover_dinos, NULL);
  pthread_create(&th_cria_dinos, NULL, criar_dinos, NULL);

  while (!fim_jogo) {
    clear();
    desenhar_entidades();
    refresh();
    wrefresh(janela_jogo.win);
    verificar_status_jogo();
    usleep(50000);
  }

  apresentar_mensagem_final();
}

int main() {
  obter_dificuldade();

  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  jogo();

  endwin();
  return 0;
}
