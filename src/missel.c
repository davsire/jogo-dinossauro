#include <stdlib.h>
#include "missel.h"

void adicionar_missel_lista(missel_t* missel, missel_t** lista_missel) {
  if (*lista_missel == NULL) {
    *lista_missel = missel;
  } else {
    missel_t* m = *lista_missel;
    while (m->prox != NULL) {
      m = m->prox;
    }
    m->prox = missel;
  }
}

void remover_missel_lista(missel_t* missel, missel_t** lista_missel) {
  missel_t *anterior, *atual = *lista_missel;

  if (atual->id == missel->id) {
    *lista_missel = atual->prox;
    free(atual);
    return;
  }

  while (atual != NULL && atual->id != missel->id) {
    anterior = atual;
    atual = atual->prox;
  }

  if (atual != NULL) {
    anterior->prox = atual->prox;
    free(atual);
  }
}
