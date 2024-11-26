#include <stdbool.h>
#include <ncurses.h>

#ifndef JOGO_DINO_H
#define JOGO_DINO_H

typedef struct JanelaJogo {
  WINDOW* win;
  int x, y;
} janela_jogo_t;

typedef struct Helicoptero {
  int x, y;
  int misseis;
} helicoptero_t;

typedef struct Deposito {
  int x, y;
  int misseis;
} deposito_t;

typedef struct Caminhao {
  int x, y;
  bool indo_deposito;
} caminhao_t;

typedef struct Dino {
  int x, y;
  int vida;
  bool ativo;
  bool indo_frente;
  bool indo_cima;
} dino_t;

void funcao();

#endif
