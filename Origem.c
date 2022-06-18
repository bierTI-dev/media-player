/*
 * Refer�ncias:
 * https://en.cppreference.com/w/c/io
 * https://en.cppreference.com/w/c/io/fscanf
 * https://en.cppreference.com/w/c/io/fgets
 * https://en.cppreference.com/w/c/string/byte/strrchr
 * E outras p�ginas do mesmo site.
 */

/*Inclusao de bibliotecas*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Defini��o de constantes*/
#define PLAYLIST_FILENAME "playlist.txt"
#define MAX_ARTIST_SIZE 51 /*Tamanho maximo de artistas em 50*/
#define MAX_TITLE_SIZE MAX_ARTIST_SIZE /*mesmo tamanho para o titulo de uma musica*/

typedef struct Song { /*Defini��o de uma musica*/
    char artist[MAX_ARTIST_SIZE]; /*Artista*/
    char title[MAX_TITLE_SIZE]; /*Titutlo da musica*/
    size_t duration[2]; /*Dura��o*/
} song_t; /*Defini��o do tipo*/

song_t* new_song(char* artist, char* title, size_t duration[2]); /*Cria��o de um novo "som"*/

typedef struct Playlist { /*Defini��o da estrutura playlist*/
    song_t* head; /*Artista, titulo e duracao*/
    struct Playlist* tail; /*Tail � nulo*/
} playlist_t;

playlist_t* new_from_file(FILE*);
playlist_t* new_playlist(song_t*);
void drop_playlist(playlist_t*); /*Libera espa�o de mem�ria*/
size_t push_song(playlist_t*, song_t*); /*Coloca um "som" na playlist*/
void print_playlist(playlist_t*); /*Printa a playlist*/

void menu(playlist_t*); /*Menu*/

int main(void) {
    FILE* file = NULL; /*Inicializa file com nullo*/
    errno_t e = fopen_s(&file, PLAYLIST_FILENAME, "r"); /*Abre o arquivo em modo leitura*/

    if (e != 0) { /*Se o erro for diferente de 0*/
        perror("Erro, nao foi possivel abrir o arquivo");
        return 1; /*Retorna erro*/
    }
    
    playlist_t* pl = new_from_file(file); /*Cria a playlist a partir do arquivo*/
    fclose(file); /*Fecha o arquivo*/
    menu(pl); /*Chama o menu com a playlist*/
    drop_playlist(pl); /*Libera espa�o que est� utilizando na playlist*/
    return 0; /*Retorna sucesso*/
}

void menu(playlist_t* pl) {
    int op;
    char line[100] = { 0 }; /*Cria um array com 100 caracteres*/
    char artist[MAX_ARTIST_SIZE] = { 0 }; /*Inicializa artista zerado*/
    char title[MAX_TITLE_SIZE] = { 0 }; /*Inicializa titulo zerado*/
    size_t duration[2]; /*Tamanho do campo dura��o :2*/

    do {
        /*Prints do menu e captura do valor digitado pelo usu�rio*/
        printf("\nSelecione uma op��o:\n");
        printf("1 - Listar m�sicas da playlist\n");
        printf("2 - Adicionar m�sica na playlist\n");
        printf("3 - Encerrar o programa\n");
        fgets(line, 5, stdin);
        if (sscanf_s(line, "%d", &op) != 1) op = 0;
        printf("\n");

        /*Switch case para executar determinada fun��o de acordo com a op��o que o usu�rio selecionar.*/
        switch (op) {
        case 1:
            print_playlist(pl);
            break;

        case 2:
            /*Grava os dados da musica*/
            printf("Digite o nome do artista: ");
            fgets(line, MAX_ARTIST_SIZE, stdin);
            if (sscanf_s(line, "%[^\n]", artist, MAX_ARTIST_SIZE) != 1) continue;

            printf("Digite o nome da m�sica: ");
            fgets(line, MAX_TITLE_SIZE, stdin);
            if (sscanf_s(line, "%[^\n]", title, MAX_TITLE_SIZE) != 1) continue;

            printf("Digite a dura��o da m�sica no formato MM:SS: ");
            fgets(line, 10, stdin);
            if (sscanf_s(line, "%zd:%zd", duration, duration + 1) != 2) continue;

            size_t len = push_song(pl, new_song(artist, title, duration));
            printf("\nM�sica adicionada; h� %d m�sicas na playlist.\n", len);
            break;

        case 3:
            break;
            /*Caso nao seja nenhuma dessas opcoes, opcao invalida.*/
        default:
            printf("Op��o inv�lida.\n");
        }


    } while (op != 3);
}

song_t* new_song(char* artist, char* title, size_t duration[2]) {
    song_t* s = calloc(sizeof(song_t), 1); /*Aloca na memoria o song_t, 118 bytes em media de acordo com o que foi declarado*/

    if (s == NULL) { /*Se o s for nullo (se nao tiver memoria, da um exit no programa)*/
        printf("N�o h� mem�ria para alocar uma nova m�sica\n");
        exit(1);
    }

    memcpy(s->artist, artist, sizeof(char) * MAX_ARTIST_SIZE); /*Faz copia da variavel artist da memoria para s_artist*/
    memcpy(s->title, title, sizeof(char) * MAX_TITLE_SIZE); /*Faz copia da variavel title da memoria para s_title*/
    memcpy(s->duration, duration, sizeof(size_t) * 2); /*Faz copia da variavel duration da memoria para s_duration*/
    return s; /*Retorna endere�o de mem�ria*/
}
/*Inicializa��o das vari�veis, tirando o "lixo" quando inicializadas*/
playlist_t* new_from_file(FILE* file) {
    char line[100];
    char artist[MAX_ARTIST_SIZE] = { 0 };
    char title[MAX_TITLE_SIZE] = { 0 };
    size_t duration[2];
    size_t assigned;

    playlist_t* pl = NULL;

    while (!feof(file)) { /*Verifica se chegou no fim do arquivo, negando*/
        if (fgets(line, 100, file) == NULL) continue; /*Verifica se ele conseguiu ler alguma linha do arquivo*/
        assigned = sscanf_s(line, "%[^-] - %[^(] (%zd:%zd)", /*l� artista, titulo e dura��o com express�o regular, ignorando hifens etc para leitura limpa*/
            artist, MAX_ARTIST_SIZE, title, MAX_TITLE_SIZE, duration, duration + 1); /*Mais um porque � um array*/
        if (assigned != 4) continue; /*Se o numero de variaveis atribuidas nao for 4, volta pro inicio do while*/

        *(strrchr(artist, ' ')) = 0; /*Recebe o artista e o caractere que estou buscando (da direita, o ultimo espa�o + uma posi��o a menos) e seta o ultimo endere�o para zero, no caso o espa�o*/
        *(strrchr(title, ' ')) = 0; /*Recebe o titulo e o caractere que estou buscando (da direita, o ultimo espa�o + uma posi��o a menos) e seta o ultimo endere�o para zero, no caso o espa�o*/

        song_t* s = new_song(artist, title, duration); /*Cria um novo som com as condicoes definidas*/

        if (pl == NULL) { /*Se S for nullo*/
            pl = new_playlist(s); /*Cria nova playlist*/
        }
        else { /*Senao, da um push no "som" passando playlist e song*/
            push_song(pl, s);
        }
    }

    return pl;
}

playlist_t* new_playlist(song_t* s) {
    /*Verifica se som = null*/
    if (s == NULL) return NULL;

    playlist_t* pl = calloc(sizeof(playlist_t), 1);

    if (pl == NULL) { /*Espa�o de mem�ria*/
        printf("N�o h� mem�ria para alocar uma nova playlist\n");
        exit(2);
    }

    pl->head = s; /*Atribui o song ao head*/
    return pl; /*Retorna playlist*/
}

void drop_playlist(playlist_t* pl) { /*Libera mem�ria*/
    if (pl == NULL) return;

    if (pl->head != NULL) {
        free(pl->head);
    }

    playlist_t* tail = pl->tail;
    free(pl);
    drop_playlist(tail);
}

size_t push_song(playlist_t* pl, song_t* s) { /*Tamanho da playlist, som*/
    if (pl == NULL) return 0;

    size_t len = 1; /*Inicializa playlist com 1*/

    while (pl->tail != NULL) { /*Enquanto o tail da playlist nao for nullo, continue indo de elemento em elemento da playlist*/
        pl = pl->tail;
        len++;
    }

    pl->tail = new_playlist(s); /*Tail = novo item na playlist*/

    return ++len;
}

void print_playlist(playlist_t* pl) { /*Print da playlist com seus resultados*/
    song_t* s;
    while (pl != NULL) {
        s = pl->head;
        printf("%s - %s (%zd:%02zd)\n", s->artist, s->title, s->duration[0], s->duration[1]);
        pl = pl->tail;
    }
}