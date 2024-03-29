#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <limits.h>

/* ----Definicoes----*/
#define N_CARROS 5
#define N_PASSAGEIROS 60
#define LIMITE_CARRO 5
/*-------------------*/


/* ----Declara��o de variaveis globais---- */

/* Vetor que contem os semaforos de cada carro:
 * - Na area de embarque
 * - Na area de desembarque*/
sem_t plataforma_embarque[N_CARROS];
sem_t plataforma_desembarque[N_CARROS];

/*Semaforos que controlam a fila de (des)embarque dos passageiros*/
sem_t fila_embarque;
sem_t fila_desembarque;

/*Semaforos que controlam a chegada na plataforma*/
sem_t chegou_plataforma;
sem_t prepararam_desembarque;

/*Semaforos que controlam a saida para o passeio*/
sem_t foi_passear;
sem_t prepararam_passeio;

/*Matriz com a imagem que eh impressa*/
char imagem[9][160];

/*Guardam quantos passageiros (des)embarcaram no carro*/
volatile int embarcaram;
volatile int prepara1,prepara2;
volatile int desembarcaram;

/*Semaforos que indicam se o carro encheu ou esvaziou*/
sem_t carro_encheu;
sem_t carro_esvaziou;

/*Mutexes que controlam o (des)embarque de um �nico passageiro por vez*/
pthread_mutex_t trava_passageiros_embarque;
pthread_mutex_t trava_preparacao1;
pthread_mutex_t trava_preparacao2;
pthread_mutex_t trava_passageiros_desembarque;

/*Semaforo que indica que alguem esta mudando o estado e imprimindo a animacao*/
sem_t mudando_estado;

/*Vetor que contem os estados de cada passageiro*/
typedef enum {AUSENTE, FILA, EMBARCANDO, NOCARRO, DESEMBARCANDO, SAIDA} estado_p;
estado_p estado_passageiros[N_PASSAGEIROS];

/*Vetor que contem os estados de cada carro*/ 
typedef enum { PASSEANDO, CARREGANDO, SAINDO, DESCARREGANDO, ESPERANDO } estado_c;
estado_c estado_carros[N_CARROS];

/*----------------------------------------*/

/*Configura a matriz imagem com o estado inicial*/
void InicializaImagem(void){
  int i,j,largura;

/* IMAGEM:
/                       Montanha Russa                                \
|            |entrada|                                                |
|            |   #__________________________________________________  |
|            |   |\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/  |
|  |ooooo|  |_____| |_____| |_____| |_____| |_____| |_____| |_____|   |
|---o---o---=o===o===o===o===o===o===o===o===o===o===o===o===o===o=---|
|                                                      |saida|   |    |
|                                                      |_________|    |
\                                                                     /
*/
																																																							  																										  		
  /*N_PASSAGEIROS+20 eh a largura da imagem*/
  if (N_PASSAGEIROS+LIMITE_CARRO+15 > 3*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+14)
	largura = N_PASSAGEIROS+LIMITE_CARRO+15;
  else
	largura = 3*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+14;

  if(largura > 150){
	printf("ERRO! valores muito grandes para serem mostrados na tela! Abortando!\n");
	exit(1);
  }
  if(largura < 25)
	largura = 40;

  for(i=1;i<largura;i++){
  	imagem[0][i] = ' ';
    imagem[1][i] = ' ';
	if(i>LIMITE_CARRO+12)
      imagem[2][i] = '_';
	else
	  imagem[2][i] = ' ';

	if(i>LIMITE_CARRO+12 && i%2 == 0){
	  imagem[3][i-1] = '\\';
	  imagem[3][i] = '/';
	}
	else
	  imagem[3][i] = ' ';

    imagem[4][i] = ' ';
	if(i<=LIMITE_CARRO+7)
	  imagem[5][i] = '-';
	else
	  imagem[5][i] = '=';
    imagem[6][i] = ' ';
    imagem[7][i] = ' ';
    imagem[8][i] = ' ';
  }
 
  imagem[0][0] = '/';
/*  imagem[0][largura/2-7] = 'M';
  imagem[0][largura/2-6] = 'o';*/
  imagem[0][largura/2-5] = '|';
  imagem[0][largura/2-4] = 'w';
  imagem[0][largura/2-3] = 'h';
  imagem[0][largura/2-2] = 'i';
  imagem[0][largura/2-1] = 'l';
  imagem[0][largura/2] = 'e';
  imagem[0][largura/2+2] = '(';
  imagem[0][largura/2+3] = '1';
  imagem[0][largura/2+4] = ')';
  imagem[0][largura/2+5] = '|';
  /*imagem[0][largura/2+6] = 'a';*/
  imagem[0][i] = '\\';
  imagem[0][i+1] = '\n';
  imagem[0][i+2] = '\0';
  
  imagem[1][0] = '|';
  imagem[1][LIMITE_CARRO+8] = '|';
  imagem[1][LIMITE_CARRO+9] = 'e';
  imagem[1][LIMITE_CARRO+10] = 'n';
  imagem[1][LIMITE_CARRO+11] = 't';
  imagem[1][LIMITE_CARRO+12] = 'r';
  imagem[1][LIMITE_CARRO+13] = 'a';
  imagem[1][LIMITE_CARRO+14] = 'd';
  imagem[1][LIMITE_CARRO+15] = 'a';
  imagem[1][LIMITE_CARRO+16] = '|';
  imagem[1][i] = '|';
  imagem[1][i+1] = '\n';
  imagem[1][i+2] = '\0';
  
  imagem[2][0] = '|';
  imagem[2][LIMITE_CARRO+8] = '|';
  imagem[2][LIMITE_CARRO+12] = '#';
  imagem[2][i-2] = ' ';
  imagem[2][i-1]   = ' ';
  imagem[2][i] = '|';
  imagem[2][i+1] = '\n';
  imagem[2][i+2] = '\0';
  
  imagem[3][0] = '|';
  imagem[3][LIMITE_CARRO+8] = '|';
  imagem[3][LIMITE_CARRO+12] = '|';
  imagem[3][i-2] = ' ';
  imagem[3][i-1]   = ' ';
  imagem[3][i] = '|';
  imagem[3][i+1] = '\n';
  imagem[3][i+2] = '\0';

  imagem[4][0] = '|';
  imagem[4][i] = '|';
  imagem[4][i+1] = '\n';
  imagem[4][i+2] = '\0';

  imagem[5][0] = '|';
  imagem[5][i-3] = '-';
  imagem[5][i-2] = '-';
  imagem[5][i-1]   = '-';
  imagem[5][i] = '|';
  imagem[5][i+1] = '\n';
  imagem[5][i+2] = '\0';
  
  imagem[6][0] = '|';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+5] = '|';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+6] = 's';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+7] = 'a';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+8] = 'i';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+9] = 'd';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+10] = 'a';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+11] = '|';
  imagem[6][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+11+LIMITE_CARRO-1] = '|';
  imagem[6][i] = '|';
  imagem[6][i+1] = '\n';
  imagem[6][i+2] = '\0';

  imagem[7][0] = '|';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+5] = '|';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+6] = '_';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+7] = '_';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+8] = '_';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+9] = '_';
  for(j=0;j<LIMITE_CARRO;j++)
    imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+10+j] = '_';
  imagem[7][2*LIMITE_CARRO+(LIMITE_CARRO+3)*N_CARROS+10+LIMITE_CARRO] = '|';
  imagem[7][i] = '|';
  imagem[7][i+1] = '\n';
  imagem[7][i+2] = '\0';
  
  imagem[8][0] = '\\';
  imagem[8][i] = '/';
  imagem[8][i+1] = '\n';
  imagem[8][i+2] = '\0';
}

void ImprimeImagem(void){
  int i;
  for(i=0; i<9; i++)
    printf("%s",imagem[i]);
}

/* ----Fun��o para realizar a animacao----*/
void* Animacao() {
  /*Variaveis que contam quantos passageiros estao num certo estado*/
  int quant_fila=0, quant_embarcando=0, quant_desembarcando=0, quant_saida=0;

  /*Variavel que conta quantos carros est�o esperando na plataforma*/
  int quant_esperando=0, existe_carregando=0, existe_saindo=0, existe_descarregando=0;
  
  /* Contador/indexador */
  int i, j;
  /* Posicao da string a partir da qual dever�o ser feitas modifica��es*/
  int offset;

  /*Conta quantos estao na fila*/
  for(i=0;i < N_PASSAGEIROS;i++){
    if(estado_passageiros[i] == FILA)
      quant_fila++;
  }

  /*Conta quantos estao embarcando*/
  for(i=0;i < N_PASSAGEIROS;i++){
    if(estado_passageiros[i] == EMBARCANDO)
      quant_embarcando++;
  }

  /*Conta quantos estao na desembarcando*/
  for(i=0;i < N_PASSAGEIROS;i++){
    if(estado_passageiros[i] == DESEMBARCANDO)
      quant_desembarcando++;
  }

  /*Conta quantos estao na saida*/
  for(i=0;i < N_PASSAGEIROS;i++){
    if(estado_passageiros[i] == SAIDA)
      quant_saida++;
  }

  /*Conta quantos carros estao esperando*/
  for(i=0;i < N_CARROS;i++){
    if(estado_carros[i] == ESPERANDO)
      quant_esperando++;
  }

  /*Checa se existe um carro carregando*/
  for(i=0;i < N_CARROS;i++){
    if(estado_carros[i] == CARREGANDO)
      existe_carregando = 1;
  }

  /*Checa se existe um carro saindo*/
  for(i=0;i < N_CARROS;i++){
    if(estado_carros[i] == SAINDO)
      existe_saindo = 1;
  }

  /*Checa se existe um carro descarregando*/
  for(i=0;i < N_CARROS;i++){
    if(estado_carros[i] == DESCARREGANDO)
      existe_descarregando = 1;
  }

  /***** Primeira Linha *****/
  /**************************/

  /***** Segunda  Linha *****/
  /**************************/

  /***** Terceira Linha *****/
  offset = LIMITE_CARRO+13;
  for(i=0;i < N_PASSAGEIROS;i++)
    if(i < quant_fila)
       imagem[2][offset+i] = 'o';
    else
       imagem[2][offset+i] = '_';
  /**************************/

  /****** Quarta Linha ******/
  /**************************/

  /****** Quinta e Sexta Linhaa ******/
  /*carro saindo*/
  offset = 3;
  if(existe_saindo){
    imagem[4][offset] = '|';
    imagem[5][offset+1] = 'o';
    for(i=0;i<LIMITE_CARRO;i++)
      imagem[4][offset+1+i] = 'o';
    imagem[4][offset+LIMITE_CARRO+1] = '|';
    imagem[5][offset+LIMITE_CARRO] = 'o';
  }
  else{
    for(i=0;i<LIMITE_CARRO+2;i++)
      imagem[4][offset+i] = ' ';
    imagem[5][offset+1] = '-';
    imagem[5][offset+LIMITE_CARRO] = '-';
  }

  /*carro sendo carregado*/
  offset += LIMITE_CARRO+4; 
  if(existe_carregando){
    imagem[4][offset] = '|';
    imagem[5][offset+1] = 'o';
    for(i=0;i < LIMITE_CARRO;i++){
      if(i < quant_embarcando)
        imagem[4][(offset+1)+i] = 'o';
      else
        imagem[4][(offset+1)+i] = '_';
    }
    imagem[4][offset+LIMITE_CARRO+1] = '|';
    imagem[5][offset+LIMITE_CARRO] = 'o';
  }
  else{
    for(i=0;i<LIMITE_CARRO+2;i++)
      imagem[4][offset+i] = ' ';
    imagem[5][offset+1] = '=';
    imagem[5][offset+LIMITE_CARRO] = '=';
  }

  /*outros carros*/
  offset += LIMITE_CARRO+3;
  for(i=0;i < N_CARROS;i++)
    if(i < quant_esperando){
       imagem[4][offset+ i*(LIMITE_CARRO+3)] = '|';
       imagem[5][(offset+1)+ i*(LIMITE_CARRO+3)] = 'o';
       for(j=0;j<LIMITE_CARRO;j++)
         imagem[4][(offset+1)+ i*(LIMITE_CARRO+3) +j] = '_';
       imagem[4][(offset+LIMITE_CARRO+1)+ i*(LIMITE_CARRO+3)] = '|';
       imagem[5][(offset+LIMITE_CARRO)+ i*(LIMITE_CARRO+3)] = 'o';
       imagem[4][(offset+LIMITE_CARRO+2)+ i*(LIMITE_CARRO+3)] = ' ';
    }
    else{
       for(j=0;j<LIMITE_CARRO+3;j++)
         imagem[4][offset+ i*(LIMITE_CARRO+3) +j] = ' ';
       imagem[5][(offset+1)+ i*(LIMITE_CARRO+3)] = '=';
       imagem[5][(offset+LIMITE_CARRO)+ i*(LIMITE_CARRO+3)] = '=';
    }

  /*carro descarregando*/
  offset += (LIMITE_CARRO+3) * N_CARROS;
  if(existe_descarregando){
    imagem[4][offset] = '|';
    for(i=0;i < LIMITE_CARRO;i++){
      imagem[5][(offset+1)+i] = ' ';  
      if(i < quant_desembarcando)
       imagem[4][(offset+1)+i] ='o';
      else
       imagem[4][(offset+1)+i] ='_';
    }
    imagem[4][offset+LIMITE_CARRO+1] = '|';
    imagem[5][offset+1] = 'o';
    imagem[5][offset+LIMITE_CARRO] = 'o';
  }
  else{
    for(i=0;i<LIMITE_CARRO+2;i++){
      imagem[4][offset+i] = ' ';
      imagem[5][offset+i] = '=';
	}
  }

  /**************************/

  /****** Setima Linha ******/
  /**************************/

  /****** Oitava Linha ******/
  for(i=0;i < LIMITE_CARRO;i++){
    if(i < quant_saida)
        imagem[7][offset+i] = 'o'; /*mantem usando o mesmo offset da linha anterior*/
    else
        imagem[7][offset+i] = '_';
  }

  /******* Nona Linha *******/
  /**************************/

  ImprimeImagem();
  
  return NULL;
}

/*----------------------------------------*/


/* ----Funcoes que serao utilizadas pelos carros---- */

/*Calcula qual eh o proximo carro*/
int proximo(int id) {
  return ( (id+1) % N_CARROS);
}

/*Carrega o carro com passageiros*/
void* carregar(int id) {
  sem_wait(&mudando_estado);
  estado_carros[id] = CARREGANDO;
  Animacao();
  printf("Carro %d esta carregando.\n\n\n",id);
  sem_post(&mudando_estado);
  sleep(rand() % 3);
  return NULL;
}

/*Descarrega o carro*/
void* descarregar(int id) {
  sem_wait(&mudando_estado);
  estado_carros[id] = DESCARREGANDO;
  Animacao();
  printf("Carro %d esta descarregando.\n\n\n",id);
  sem_post(&mudando_estado);
  sleep(rand() % 3);
  return NULL;
}

/*O carro passeia pelos trilhos*/
void* passeia() {
  /*O tempo do passeio varia entre 3 e 5 segundos*/
  sleep((rand() % 3) + 3);
  return NULL;
}

/*Funcao principal que controla os carros*/
void* Carro(void *v) {

  int k;
  int id = (int) v;

  while(1){
    /*Primeiramente o carro espera a permissao para carregar*/
    sem_wait(&plataforma_embarque[id]);

    /*Se conseguir, o imprime na area de carregamento*/
    carregar(id);

    /*Permite a passagem de LIMITE_CARROS passageiros e espera encher*/
    for(k=0;k < LIMITE_CARRO; k++)
      sem_post(&fila_embarque);
    sem_wait(&carro_encheu);

    /*Imprime o carro saindo e desaparecendo*/
    sem_wait(&mudando_estado);
    estado_carros[id] = SAINDO;
    Animacao();
    printf("Carro %d esta saindo.\n\n\n",id);
    estado_carros[id] = PASSEANDO;
    Animacao();
    printf("Carro %d esta passeando.\n\n\n",id);
    /*Prepara LIMITE_CARROS passageiros para passear*/
    for(k=0;k < LIMITE_CARRO; k++)
      sem_post(&foi_passear);
    sem_wait(&prepararam_passeio);
    sem_post(&mudando_estado);

    /*Permite que outro carro carregue e passeia*/
    sem_post(&plataforma_embarque[proximo(id)]);
    passeia();

    /*Espera a area de desembarque ficar livre*/
    sem_wait(&plataforma_desembarque[id]);

    /*Se ficou, prepara LIMITE_CARROS passageiros para desembarcar*/
    for(k=0;k < LIMITE_CARRO; k++)
      sem_post(&chegou_plataforma);
    sem_wait(&prepararam_desembarque);

    /*Imprime o carro na area de desembarque*/
    descarregar(id);

    /*Libera a saida de LIMITE_CARROS passageiros e espera esvaziar*/
    for(k=0;k < LIMITE_CARRO; k++)
      sem_post(&fila_desembarque);
    sem_wait(&carro_esvaziou);

    /*Depois de esvaziar, volta a fila de espera*/
    sem_wait(&mudando_estado);
    estado_carros[id] = ESPERANDO;
    Animacao();
    printf("Carro %d esta esperando.\n\n\n",id);
    sem_post(&mudando_estado);

    /*Libera a plataforma de desembarque para outro carro*/
    sem_post(&plataforma_desembarque[proximo(id)]);
  }
  
  return NULL;
}

/*---------------------------------------------------*/



/* ----Funcoes que serao utilizadas pelos passageiros---- */

/*Embarca o passageiro*/
void* embarcar(int id) {
  sem_wait(&mudando_estado);
  estado_passageiros[id] = EMBARCANDO;
  Animacao();
  printf("Passageiro %d embarcou.\n\n\n",id);
  sem_post(&mudando_estado);
  sleep(rand() % 3);
  return NULL;
}

/*Desembarca o passageiro*/
void* desembarcar(int id) {
  sem_wait(&mudando_estado);
  estado_passageiros[id] = SAIDA;
  Animacao();
  printf("Passageiro %d desembarcou.\n\n\n",id);
  sem_post(&mudando_estado);
  sleep(rand() % 3);
  return NULL;
}

/*Fun��o principal que controla os passageiros*/
void* Passageiro(void *v) {
  int id = (int) v;

  /*Para tornar mais real, os passageiros demoram um tempo aleatorio para surgir*/
  sleep(rand() % N_PASSAGEIROS);

  /*Mostra que o passageiro esta na fila*/
  sem_wait(&mudando_estado);
  estado_passageiros[id] = FILA;
  Animacao();
  printf("Passageiro %d esta na fila.\n\n\n",id);
  sem_post(&mudando_estado);

  /*O passageiro espera a entrada no carro*/
  sem_wait(&fila_embarque);
  
  /*Se esta for permitida, imprime o passageiro embarcando*/
  embarcar(id);

  /*Os passageiros vao entrando e avisam quando encher o carro*/
  pthread_mutex_lock(&trava_passageiros_embarque);
    embarcaram += 1;
    if (embarcaram == LIMITE_CARRO){
      sem_post(&carro_encheu);
      embarcaram = 0;
    }
  pthread_mutex_unlock(&trava_passageiros_embarque);

  /*Preparam os passageiros para o passeio*/
  sem_wait(&foi_passear);
  pthread_mutex_lock(&trava_preparacao1);
    estado_passageiros[id] = NOCARRO;
    prepara1 += 1;
    if (prepara1 == LIMITE_CARRO){
      sem_post(&prepararam_passeio);
      prepara1 = 0;
    }
  pthread_mutex_unlock(&trava_preparacao1);

  /*Prepara os passageiros para desembarcar*/
  sem_wait(&chegou_plataforma);
  pthread_mutex_lock(&trava_preparacao2);
    sem_wait(&mudando_estado);
    estado_passageiros[id] = DESEMBARCANDO;
    sem_post(&mudando_estado);
    prepara2 += 1;
    if (prepara2 == LIMITE_CARRO){
      sem_post(&prepararam_desembarque);
      prepara2 = 0;
    }
  pthread_mutex_unlock(&trava_preparacao2);

  /*Espera para poder desembarcar*/
  sem_wait(&fila_desembarque);

  /*Quando puder, imprime o passageiro na saida*/
  desembarcar(id);

  /*Os passageiros saem do carro e avisam quando ele esvaziar*/
  pthread_mutex_lock(&trava_passageiros_desembarque);
    desembarcaram += 1;
    if (desembarcaram == LIMITE_CARRO){
      sem_post(&carro_esvaziou);
      desembarcaram = 0;
    }
  pthread_mutex_unlock(&trava_passageiros_desembarque);

  /*O passageiro vai embora*/
  sem_wait(&mudando_estado);
  estado_passageiros[id] = AUSENTE;
  sem_post(&mudando_estado);
  return NULL;
}

/*------------------------------------------------------- */


int main() {

  /*Vetores de threads de passageiros e carros*/
  pthread_t passageiro[N_PASSAGEIROS];
  pthread_t carro[N_PASSAGEIROS]; 

  int i;
  
  /*Inicializa os semaforos comuns*/
  sem_init(&fila_embarque,0,0);
  sem_init(&fila_desembarque,0,0);
  sem_init(&prepararam_desembarque,0,0);
  sem_init(&chegou_plataforma,0,0);
  sem_init(&carro_encheu,0,0);
  sem_init(&carro_esvaziou,0,0);
  sem_init(&mudando_estado,0,1);


  /*Inicializa os vetores de semaforos*/
  sem_init(&plataforma_embarque[0],0,1);
  for(i=1;i < N_CARROS; i++)
    sem_init(&plataforma_embarque[i],0,0);
  sem_init(&plataforma_desembarque[0],0,1);
  for(i=1;i < N_CARROS; i++)
    sem_init(&plataforma_desembarque[i],0,0);
  
  /*Todos os passageiros comecam no estado ausente*/
  for(i=0;i < N_PASSAGEIROS;i++)
    estado_passageiros[i] = AUSENTE;

  /*Todos os carros comecam no estado esperando*/
  for(i=0;i < N_CARROS;i++)
    estado_carros[i] = ESPERANDO;

  /*Imprime a situacao inicial da animacao*/
  InicializaImagem();
  Animacao();
  printf("Imprimiu situacao inicial\n\n\n");

  /*Cria os carros*/
  for (i = 0; i < N_CARROS; i++) 
    if(pthread_create(&carro[i], NULL, Carro, (void*) i))
      printf("Erro ao criar carro!\n\n");

  /*Cria os passageiros*/
  for (i = 0; i < N_PASSAGEIROS; i++) 
    if(pthread_create(&passageiro[i], NULL, Passageiro, (void*) i))
      printf("Erro ao criar passageiro!\n\n");

  /*Espera os passageiros terminarem*/
  for (i = 0; i < N_PASSAGEIROS; i++) 
    if(pthread_join(passageiro[i], NULL))
      printf("Erro ao esperar o passageiro!\n\n"); 

  /*Espera os carros terminarem*/
  sleep(4);

  /*Mata as threads de carro*/ 

  return 0;
}

