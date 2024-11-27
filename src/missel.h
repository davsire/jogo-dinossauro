#include <pthread.h>

#ifndef MISSEL_H
#define MISSEL_H

typedef struct Missel {
  int id;
  int x, y;
  bool acertou;
  pthread_t thread;
  struct Missel* prox;
} missel_t;

void adicionar_missel_lista(missel_t* missel, missel_t** lista_missel);

void remover_missel_lista(missel_t* missel, missel_t** lista_missel);

#endif
