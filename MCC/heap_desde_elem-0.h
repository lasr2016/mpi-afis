/*
* Copyright (C) 2010 Ricardo Javier Barrientos Rojel (ricardo.j.barrientos@gmail.com)
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License (http://www.gnu.org/licenses/gpl.txt)
* as published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
**********/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// int N; //usado por main()

struct _Elem
{
    int ind;
    float dist;
};
typedef struct _Elem Elem;

struct _ElemSL
{
    float dist;
    int i;
    int j;
};
typedef struct _ElemSL ElemSL;

/* Funciones  */

//inserta2 y extrae2 usan el arreglo del heap desde el elemento 0
void inserta2(int *heap, int elem, int *n_elem);
int extrae2(int *heap, int *n_elem);

//inserta y extrae usan el arreglo del heap desde el elemento 1
void inserta(int *heap, int elem, int *n_elem);
int extrae(int *heap, int *n_elem);

//Extrae y agregar en una sola funciÃon, evitando algunos pasos innecesarios aplicados cuando se extrae y agrega por separado. La raiz mantiene el mayor de todos los elementos
void popush2E(Elem *heap, int *n_elem, Elem *elem);
void inserta2E(Elem *heap, Elem *elem, int *n_elem);
Elem extrae2E(Elem *heap, int *n_elem);

//Igual que los anteriores, pero con la raiz como el menor de todos los elementos
void popush2E_min(Elem *heap, int *n_elem, Elem *elem);
void inserta2E_min(Elem *heap, Elem *elem, int *n_elem);
Elem extrae2E_min(Elem *heap, int *n_elem);

//Funciones para el tipo ElemSL
void inserta2ESLmin(ElemSL *heap, ElemSL *elem, int *n_elem);
void popush2ESLmin(ElemSL *heap, int *n_elem, ElemSL *elem);

void popush2f(float *heap, int *n_elem, float elem);
void inserta2f(float *heap, float elem, int *n_elem);
int extrae2f(float *heap, int *n_elem);

//Extrae y agregar en una sola funciÃon, evitando algunos pasos innecesarios aplicados cuando se extrae y agrega por separado
void popush2E(Elem *heap, int *n_elem, Elem *elem)
{
  int i, k;
  Elem temp;
 
  heap[0].dist = elem->dist;
  heap[0].ind = elem->ind;

  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1].dist > heap[k-1].dist)
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1].dist > heap[k-1].dist)
      break;  //es mayor que ambos hijos

    temp = heap[i-1];
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return;
}

//inserta2E es igual que inserta2, pero con elementos Elem. La raiz es el mayor de todos los elementos
void inserta2E(Elem *heap, Elem *elem, int *n_elem)
{
  int i;
  Elem temp;
  
  heap[*n_elem].ind = elem->ind;
  heap[*n_elem].dist = elem->dist;
  (*n_elem)++;

  for (i = *n_elem; i>1 && heap[i-1].dist > heap[(i/2)-1].dist; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i-1];
    heap[i-1] = heap[(i/2)-1];
    heap[(i/2)-1] = temp;
  }
}

//inserta2E es igual que inserta2, pero con elementos Elem. La raiz es el mayor de todos los elementos
Elem extrae2E(Elem *heap, int *n_elem)
{
  int i, k;
  Elem temp, max;
 
  max = heap[0];  

  heap[0] = heap[(*n_elem)-1];  // Movemos el ultimo a la raiz y achicamos el heap
  (*n_elem)--;
  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1].dist > heap[k-1].dist)
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1].dist > heap[k-1].dist)
      break;  //es mayor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return max;
}

void popush2E_min(Elem *heap, int *n_elem, Elem *elem)
{
  int i, k;
  Elem temp;
 
  heap[0].dist = elem->dist;
  heap[0].ind = elem->ind;

  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1].dist < heap[k-1].dist)
      k = k+1;  //el hijo derecho es el menor
    if(heap[i-1].dist < heap[k-1].dist)
      break;  //es menor que ambos hijos

    temp = heap[i-1];
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return;
}

void inserta2E_min(Elem *heap, Elem *elem, int *n_elem)
{
  int i;
  Elem temp;
  
  heap[*n_elem].ind = elem->ind;
  heap[*n_elem].dist = elem->dist;
  (*n_elem)++;

  for (i = *n_elem; i>1 && heap[i-1].dist < heap[(i/2)-1].dist; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i-1];
    heap[i-1] = heap[(i/2)-1];
    heap[(i/2)-1] = temp;
  }
  return;
}

Elem extrae2E_min(Elem *heap, int *n_elem)
{
  int i, k;
  Elem temp, max;
 
  max = heap[0];  

  heap[0] = heap[(*n_elem)-1];  // Movemos el ultimo a la raiz y achicamos el heap
  (*n_elem)--;
  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1].dist < heap[k-1].dist)
      k = k+1;  //el hijo derecho es el menor
    if(heap[i-1].dist < heap[k-1].dist)
      break;  //es menor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return max;
}

//popush2f es la version de popush2 para elementos float
void popush2f(float *heap, int *n_elem, float elem)
{
  int i, k;
  float temp;
 
  heap[0] = elem;

  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1] > heap[k-1])
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1] > heap[k-1])
      break;  //es mayor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return;
}

//inserta2 y extrae2 usan el arreglo del heap desde el elemento 0
void inserta2(int *heap, int elem, int *n_elem)
{
  int i, temp;
  
  heap[*n_elem] = elem;
  (*n_elem)++;
  for (i = *n_elem; i>1 && heap[i-1]>heap[(i/2)-1]; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i-1];
    heap[i-1] = heap[(i/2)-1];
    heap[(i/2)-1] = temp;
  }
}

//inserta2 y extrae2 usan el arreglo del heap desde el elemento 0
int extrae2(int *heap, int *n_elem)
{
  int i, k, temp, max = heap[0];  //La variable m lleva el maximo

  heap[0] = heap[(*n_elem)-1];  // Movemos el ultimo a la raiz y achicamos el heap
  (*n_elem)--;
  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1] > heap[k-1])
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1] > heap[k-1])
      break;  //es mayor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return max;
}

//inserta2f y extrae2f usan el arreglo del heap de elementos float desde el elemento 0
void inserta2f(float *heap, float elem, int *n_elem)
{
  int i;
  float temp;
  
  heap[*n_elem] = elem;
  (*n_elem)++;
  for (i = *n_elem; i>1 && heap[i-1]>heap[(i/2)-1]; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i-1];
    heap[i-1] = heap[(i/2)-1];
    heap[(i/2)-1] = temp;
  }
  return;
}

int extrae2f(float *heap, int *n_elem)
{
  int i, k;
  float temp, max = heap[0];  //La variable m lleva el maximo

  heap[0] = heap[(*n_elem)-1];  // Movemos el ultimo a la raiz y achicamos el heap
  (*n_elem)--;
  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1] > heap[k-1])
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1] > heap[k-1])
      break;  //es mayor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return max;
}

//inserta y extrae usan el arreglo del heap desde el elemento 1
void inserta(int *heap, int elem, int *n_elem)
{
  int i, temp;
  
  (*n_elem)++;
  heap[*n_elem] = elem;
  for (i = *n_elem; i>1 && heap[i]>heap[i/2]; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i];
    heap[i] = heap[i/2];
    heap[i/2] = temp;
  }
}

int extrae(int *heap, int *n_elem)
{
  int i, k, temp, max = heap[1];  //La variable m lleva el maximo

  heap[1] = heap[*n_elem];  // Movemos el ultimo a la raiz y achicamos el heap
  (*n_elem)--;
  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[k+1] > heap[k])
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i] > heap[k])
      break;  //es mayor que ambos hijos

    temp = heap[i];  
    heap[i] = heap[k];
    heap[k] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return max;
}


//inserta2ESLmin es igual que inserta2, pero con elementos Elem. La raiz es el menor valor de todos los elementos
void inserta2ESLmin(ElemSL *heap, ElemSL *elem, int *n_elem)
{
  int i;
  ElemSL temp;
  
  heap[*n_elem].i = elem->i;
  heap[*n_elem].j = elem->j;
  heap[*n_elem].dist = elem->dist;
  (*n_elem)++;

  for (i = *n_elem; i>1 && heap[i-1].dist < heap[(i/2)-1].dist; i=i/2)
  {
    //Intercambiamos con el padre
    temp = heap[i-1];
    heap[i-1] = heap[(i/2)-1];
    heap[(i/2)-1] = temp;
  }
  return;
}

//popush2f es la version de popush2 para elementos ElemSL. La raiz es el menor de todos los elementos del heap.
void popush2ESLmin(ElemSL *heap, int *n_elem, ElemSL *elem)
{
  int i, k;
  ElemSL temp;
 
  heap[0].dist = elem->dist;
  heap[0].i = elem->i;
  heap[0].j = elem->j;

  i = 1;
  while(2*i <= *n_elem) // mientras tenga algun hijo
  {
    k = 2*i; //el hijo izquierdo
    if(k+1 <= *n_elem && heap[(k+1)-1].dist < heap[k-1].dist)
      k = k+1;  //el hijo derecho es el mayor
    if(heap[i-1].dist < heap[k-1].dist)
      break;  //es mayor que ambos hijos

    temp = heap[i-1];  
    heap[i-1] = heap[k-1];
    heap[k-1] = temp;  
    i = k;   //lo intercambiamos con el mayor hijo
  }
  return;
}

/*
main(int argc, char *argv[])
{
  int i, *heap, n_elem=0;
  srand(time(NULL));
  N = atoi(argv[1]);
  heap = (int *)malloc(sizeof(int)*(N+1));
  
//  for (i=0; i<N; i++)
//    inserta(heap, rand()%100, &n_elem);

//  for (i=0; i<N; i++)
//    printf("extrae() = %d\n", extrae(heap, &n_elem));

//    printf("\n----------------\n");

  for (i=0; i<N; i++)
    inserta2(heap, rand()%100, &n_elem);
  for (i=0; i<N/2; i++)
    printf("extrae2() = %d\n", extrae2(heap, &n_elem));
  for (i=0; i<N/2; i++)
    inserta2(heap, rand()%100, &n_elem);

  for (i=0; i<N; i++)
    printf("heap[%d] = %d\n", i, heap[i]);
  printf("n_elem = %d\n", n_elem);
  printf("\n-----\n");

  for (i=0; i<N; i++)
    printf("extrae2() = %d\n", extrae2(heap, &n_elem));
  printf("n_elem = %d\n", n_elem);
}
*/



