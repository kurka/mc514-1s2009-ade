/* 
 * Campeonato entre 4 threads para acesso �regi�o cr�tica.
 *
 * Modifique este c�digo para funcionar para N threads,
 * com apenas uma fun��o f_thread.
 *
 * Deve ser colocado um comando sleep nos seguintes pontos:
 *   - entre as partidas;
 *   - entre a atribui��o � vari�vel s e a impress�o do
 *     valor desta vari�vel;
 *   - fora da regi�o cr�tica.  
 *
 * Substitua as esperas ocupadas por chamadas a futex_wait. 
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

/* Retorna -1 se o futex n�o bloqueou e 
            0 caso contr�rio */
int futex_wait(void *addr, int val1) {
  return syscall(SYS_futex, addr, FUTEX_WAIT, 
                 val1, NULL, NULL, 0);
}

/* Retorna o n�mero de threads que foram acordadas */
int futex_wake(void *addr, int n) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, 
                   n, NULL, NULL, 0);
}

/* N�mero de vezes que uma thread deve entrar na regi�o cr�tica. */
#define N_VEZES 15
#define N_THR 4
/* Vari�vel compartilhada */
volatile int s = 0;

/* Vari�veis de controle para a final. */
int ultimo_final = 0;
//int interesse_final[2] = { 0, 0 };

/* Vari�eis de controle para a partida
   entre a thread 0 e a thread 1.      */
int ultimo_01 = 0;
int interesse_01[2] = { 0, 0 };

/* Vari�veis de controle para a partida
   entre a thread 2 e a thread 3.      */
int ultimo_23 = 0;
int interesse_23[2] = { 0, 0 };

int penultimo[2] = {0, 0};
int interesse_semi[4] = {0, 0, 0, 0};
int interesse_final[2] = {0, 0};
int ultimo = 0;

/*Fun��o que acha o rival de uma thread em uma disputa (de acordo com o id)*/
int rival(int thr_id){
	if(thr_id%2 == 0) /*se for par, disputa com a pr�xima (ex: 0 disputa com thr_id 1)*/
		return thr_id+1;
	else	/*caso contr�rio, disputa com a anterior*/
		return thr_id-1;
}

/* Fun��o gen�rica para as threads*/
void* f_thread(void *v) {
  int i, thr_id;
//  int tmp;
  thr_id = *(int *) v;

  
  for (i = 0; i < N_VEZES; i++) {

    interesse_semi[thr_id] = 1;
    penultimo[thr_id/2] = thr_id;

    while (penultimo[thr_id/2] == thr_id && interesse_semi[rival(thr_id)]) ; 

    sleep(1); /* Sleep entre as partidas */
     
    interesse_final[thr_id/2] = 1; //fix
    ultimo = thr_id;
    
    while (ultimo == thr_id && interesse_final[rival(thr_id/2)]) ; 

    s = thr_id;
    sleep(1); /* Sleep entre a atribui��o e a impress�o */    
    printf("Thread %d, s = %d.\n", thr_id, s); 

    interesse_final[thr_id/2] = 0;    
    interesse_semi[thr_id] = 0;

    sleep(1); /* Sleep fora da regi�o cr�tica */
  }

  return NULL;
}

/* Fun��o para a thread 0*/
void* f_thread_0(void *v) {
  int i;
  
  for (i = 0; i < N_VEZES; i++) {

    interesse_01[0] = 1;
    ultimo_01 = 0;
    while (ultimo_01 == 0 && interesse_01[1]) ; 

    sleep(1); /* Sleep entre as partidas */
     
    interesse_final[0] = 1;
    ultimo_final = 0;
    
    while (ultimo_final == 0 && interesse_final[1]) ; 

    s = 0;
    sleep(1); /* Sleep entre a atribui��o e a impress�o */    
    printf("Thread 0, s = %d.\n", s); 

    interesse_final[0] = 0;    
    interesse_01[0] = 0;

    sleep(1); /* Sleep fora da regi�o cr�tica */
  }

  return NULL;
}

/* Fun��o para a thread 1 */
void* f_thread_1(void *v) {
  int i;
  
  for (i = 0; i < N_VEZES; i++) {

    interesse_01[1] = 1;
    ultimo_01 = 1;
    while (ultimo_01 == 1 && interesse_01[0]) ; 

    sleep(1); /* Sleep entre as partidas */
    
    interesse_final[0] = 1;
    ultimo_final = 1;
    
    while (ultimo_final == 1 && interesse_final[1]) ; 

    s = 1;
    sleep(1); /* Sleep entre a atribui��o e a impress�o */     
    printf("Thread 1, s = %d.\n", s); 

    interesse_final[0] = 0;    
    interesse_01[1] = 0;

    sleep(1); /* Sleep fora da regi�o cr�tica */    
  }

  return NULL;
}

/* Fun��o para a thread 2 */
void* f_thread_2(void *v) {
  int i;
  
  for (i = 0; i < N_VEZES; i++) {

    interesse_23[0] = 1;
    ultimo_23 = 2;
    while (ultimo_23 == 2 && interesse_23[1]) ; 

    sleep(1); /* Sleep entre as partidas */
    
    interesse_final[1] = 1;
    ultimo_final = 2;
    while (ultimo_final == 2 && interesse_final[0]) ; 

    s = 2;
    sleep(1); /* Sleep entre a atribui��o e a impress�o */        
    printf("Thread 2, s = %d.\n", s); 

    interesse_final[1] = 0;    
    interesse_23[0] = 0;

    sleep(1); /* Sleep fora da regi�o cr�tica */    
  }

  return NULL;
}

/* Fun��o para a thread 3 */
void* f_thread_3(void *v) {
  int i;
  
  for (i = 0; i < N_VEZES; i++) {

    interesse_23[1] = 1;
    ultimo_23 = 3;
    while (ultimo_23 == 3 && interesse_23[0]) ; 

    sleep(1); /* Sleep entre as partidas */
    
    interesse_final[1] = 1;
    ultimo_final = 3;
    while (ultimo_final == 3 && interesse_final[0]) ; 

    s = 3;
    sleep(1);    
    printf("Thread 3, s = %d.\n", s); 

    interesse_final[1] = 0;    
    interesse_23[1] = 0;

    sleep(1); /* Sleep fora da regi�o cr�tica */    
  }

  return NULL;
}

int main() {

//  pthread_t thr0, thr1, thr2, thr3;

   pthread_t thr[N_THR];
   int i, id[N_THR];
	   
   for (i = 0; i < N_THR; i++) {
  	 id[i] = i;
     pthread_create(&thr[i], NULL, f_thread, (void*) &id[i]);
	  }

   for (i = 0; i < N_THR; i++) 
	   pthread_join(thr[i], NULL); 


/*
  pthread_create(&thr0, NULL, f_thread_0, NULL);
  pthread_create(&thr1, NULL, f_thread_1, NULL);
  pthread_create(&thr2, NULL, f_thread_2, NULL);
  pthread_create(&thr3, NULL, f_thread_3, NULL);

  pthread_join(thr0, NULL); 
  pthread_join(thr1, NULL); 
  pthread_join(thr2, NULL); 
  pthread_join(thr3, NULL); 
*/
  return 0;
}

