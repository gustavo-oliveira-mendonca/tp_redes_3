#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include "tp_socket.h"
#include "fsmCliente.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>

// #include <unistd.h>
// #include <arpa/inet.h> //INET_NTOP
// #include <stdbool.h>

#define DEBUG

#define TAM_PORTA 6
#define TAM_NOME_ARQUIVO 32
#define TAM_HOST 16

long getTime();
long timeDiff(long, long);
void carregaParametros(int*, char**,  char*, short int*, char*, int*);
void estadoEnviaReq(int*);
void estadoRecebeArq(int*);
void estadoErro(int*);
void estadoEnviaAck(int*);      
void estadoTermino(int*);

int sockCliFd, mtu;
pacote *envio;
transacao *t;
char* buf; // TODO: avaliar se buffer pode ser transferido para dentro da struct transacao
short int porta;
char host[TAM_HOST];
so_addr *saddr;

int main(int argc, char* argv[]){
  int tamBuffer;
  // long int bytesRecebidos = 0;
  char nomeArquivo[TAM_NOME_ARQUIVO];
  
  // double comeco, duracao;

  // alimenta numero da porta e tamanho do buffer pelos parametros recebidos
  carregaParametros(&argc, argv, host, &porta, nomeArquivo, &tamBuffer);


  
  // aloca memória para buffer
  buf = malloc(sizeof *buf * tamBuffer);
  if (buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }

  saddr = malloc(sizeof(so_addr));

  // chamada de função de inicialização para ambiente de testes
  tp_init();

  // cria socket e armazena o respectivo file descriptor
  sockCliFd = tp_socket(0);

  int estadoAtual = ESTADO_ENVIA_REQ;
  int operacao; 

  envio = criaPacoteVazio();
  t = criaTransacaoVazia();
  strcpy(t->nomeArquivo, nomeArquivo);
  while(1){
    switch(estadoAtual){
      case ESTADO_ENVIA_REQ:
        estadoEnviaReq(&operacao);
        break;
      case ESTADO_RECEBE_ARQ:
        estadoRecebeArq(&operacao);
        break;
      case ESTADO_ERRO:
        estadoErro(&operacao);
        break;
      case ESTADO_ENVIA_ACK:
        estadoEnviaAck(&operacao);
        break;
      case ESTADO_TERMINO:
        estadoTermino(&operacao);
      break;
    }
    transita(&estadoAtual, &operacao);
  }

  free(saddr);
  exit(EXIT_SUCCESS);
}

void estadoEnviaReq(int *operacao){
  int socket, status;
  // TODO: verificar possibilidade de criar pacote 'envio', enviar e excluí-lo aqui dentro desta função
  envio->opcode = REQ;
  strcpy(envio->nomeArquivo, t->nomeArquivo);
  montaBufferPeloPacote(buf, envio);

  // cria socket
  socket = tp_socket(porta);

  // forma endereço para envio do pacote ao servidor
  if (tp_build_addr(saddr, host, porta) < 0){
      perror("Erro no envio do pacote de requisicao de arquivo");
      // TODO: tratar erro
  }

  status = tp_sendto(socket, buf, mtu, saddr);
  // verifica estado do envio
    if (status > 0) {
      *operacao = OPERACAO_OK;
    } else {
      *operacao = OPERACAO_NOK;
    }
    destroiPacote(envio);
}

void estadoRecebeArq(int *operacao){

}

void estadoErro(int *operacao){

}

void estadoEnviaAck(int *operacao){

}

void estadoTermino(int *operacao){

}

// UTIL
void carregaParametros(int* argc, char** argv, char* host, short int* porta, char* arquivo, int* tamBuffer){
  #ifdef DEBUG
    printf("[DEBUG] numero de parametros recebidos: %d\n", *argc);
  #endif
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 5){
    fprintf(stderr, "ERRO-> parametros invalidos! Uso: %s [host] [porta] [nome_arquivo] [tam_buffer]\n", argv[0]);
    exit(EXIT_FAILURE);
  } 
  else {
    errno = 0;
    *tamBuffer = strtoul(argv[4], &ultimoCaractere, 10);
    if ((errno == ERANGE && (*tamBuffer == LONG_MAX || *tamBuffer == LONG_MIN)) || (errno != 0 && *tamBuffer == 0)){
      perror("strtoul");
      exit(EXIT_FAILURE);
    }
  }
  *porta = atoi(argv[2]);

  memset(host, '\0', TAM_HOST);
  strcpy(host, argv[1]);

  memset(arquivo, '\0', TAM_NOME_ARQUIVO);
  strcpy(arquivo, argv[3]);
  
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> host: %s porta: %d nome_arquivo: %s tamBuffer: %d\n", host, *porta, arquivo, *tamBuffer);
  #endif
}

long getTime(){
    struct timespec tempo;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tempo);
    return tempo.tv_nsec;
}
  
// retorna diferença em nanosegundos
long timeDiff(long start, long end){
  long temp;
  if ((end - start) < 0){
    temp = 1000000000 + end - start;
  } else {
    temp = end - start;
  }
  return temp;
}