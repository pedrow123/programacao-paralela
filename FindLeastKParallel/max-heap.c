#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "chrono.h"

#define THREAD_MIN 1
#define THREAD_MAX 8
#define SHOW_DECREASE_MAX_STEPS 1
#define MAX_HEAP_SIZE (1024 * 1024)

typedef struct
{
    float chave;
    int valor;
} par_t;

int n, k, nThreads, tamOutput;
float *input;
par_t *output;
par_t **heaps;
int* tamHeaps;

void drawHeapTree(par_t heap[], int size, int nLevels) // FIX ME!
{
    int offset = 0;
    int space = (int)pow(2, nLevels - 1);

    int nElements = 1;
    for (int level = 0; level < nLevels; level++)
    {
        // print all elements in this level
        for (int i = offset; i < size && i < (offset + nElements); i++)
            printf("[chave: %.0f, índice: %d]", heap[i].chave, heap[i].valor);
        printf("\n");

        offset += nElements;
        space = nElements - 1;
        nElements *= 2;
    }
}

void swap(par_t *a, par_t *b)
{
    par_t temp = *a;
    *a = *b;
    *b = temp;
}

void maxHeapify(par_t heap[], int tam, int i)
{
    while (1)
    {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < tam && heap[left].chave > heap[largest].chave)
            largest = left;

        if (right < tam && heap[right].chave > heap[largest].chave)
            largest = right;

        if (largest != i)
        {
            swap(&heap[i], &heap[largest]); // Using the swap function
            i = largest;
        }
        else
            break;
    }
}

// #define parent(pos) ( pos/2 ) // SE nao usar posicao 0
#define parent(pos) ((pos - 1) / 2)

void heapifyUp(par_t* heap, int pos)
{
    float chave = heap[pos].chave;
    float valor = heap[pos].valor;

    while (pos > 0 && chave > heap[parent(pos)].chave)
    {
        heap[pos].chave = heap[parent(pos)].chave;
        heap[pos].valor = heap[parent(pos)].valor;
        pos = parent(pos);
    }

    heap[pos].chave = chave;
    heap[pos].valor = valor;
}

void insert(par_t* heap, int *tam, float chave, int valor)
{
    *tam += 1;
    int last = *tam - 1;

    heap[last].chave = chave;
    heap[last].valor = valor;

    //printf("Inserido --> %1f\n", heap[last].chave);

    heapifyUp(heap, last);
}

int isMaxHeap(par_t heap[], int tam)
{
    for (int i = 1; i < tam; i++)
        if (heap[i].chave <= heap[parent(i)].chave)
            continue;
        else
        {
            printf("\nbroke at [%d]=%f\n", i, heap[i].chave);
            printf("father at [%d]=%f\n", parent(i), heap[parent(i)].chave);
            return 0;
        }
    return 1;
}

void decreaseMax(par_t heap[], int tam, float chave, int valor)
{
    if (tam == 0) // Heap is empty
        return;

    if (heap[0].chave > chave)
    {
        heap[0].chave = chave;
        heap[0].valor = valor;

        // #if SHOW_DECREASE_MAX_STEPS
        //     // drawHeapTree( heap, tam, 4 );
        //     printf("    ~~~~~~~~~~~~~~~~~~~~~~~~~DECREASE MAX!\n");
        // #endif

        maxHeapify(heap, tam, 0);
    }
}

void geraNaleatorios(float v[], int n)
{
    for(int i = 0; i < n; i++)
    {
        // Gera dois aleatorios entre 0 e RAND_MAX
	    int a = rand();
	    int b = rand(); 
	
        // Junta esses dois e forma um numero para o vetor v
	    float elem = a * 100.0 + b;

        // inserir o valor v na posição p
	    v[i] = elem;
    }
}

void *acharKMenores(void *arg) 
{
    int threadNum = (int)arg;
    int indexPrimElem = threadNum * (n/nThreads);  // Cada thread vai ter n/nthreads elementos
    int indexUltElem;                              // A multiplicação é pra ver em qual dos blocos ela está

    // Checa se é a última thread
    if (threadNum == nThreads - 1)
        indexUltElem = n - 1;
    else
    {
        if (((threadNum + 1) * (n/nThreads)) < n)
            indexUltElem = (threadNum + 1) * (n/nThreads) - 1;
        else
            indexUltElem = n - 1;
    }

    // Vai inserindo na heap 
    for (int i = indexPrimElem; i <= indexUltElem; i++)
    {
        if (tamHeaps[threadNum] < k)
            insert(heaps[threadNum], &tamHeaps[threadNum], input[i], i);
        else
            decreaseMax(heaps[threadNum], k, input[i], i);          
    }

    pthread_exit(NULL);
    return NULL;
}

// Necessário pra usar o qsort
int compare(const void *a, const void *b) 
{
    const par_t *parA = (const par_t *)a;
    const par_t *parB = (const par_t *)b;

    if (parA->chave < parB->chave) return -1;
    if (parA->chave > parB->chave) return 1;
    
    // Retorna 0 se os elementos são iguais
    return 0;
}

void verifyOutput(const float *Input, const par_t *Output, int nTotalElmts, int k)
{
    par_t* I = malloc(nTotalElmts * sizeof(par_t));
    int ok = 1;

    for (int i = 0; i < nTotalElmts; i++)
    {
        I[i].chave = Input[i];
        I[i].valor = i;
    }

    // Ordena I
    qsort(I, nTotalElmts, sizeof(par_t), compare);

    // Ordena Output -- Pra ficar mais rápido a verificação
    qsort((void*) Output, k, sizeof(par_t), compare);

    for (int i = 0; i < k; i++)
    {
        if (I[i].valor != Output[i].valor)
        {
            // Caso de elementos iguais em ordem oposta
            // Podia resolver isso usando um alg de ordenacao estavel
            if (I[i].chave != Output[i].chave)
            {
                ok--;
                break;
            }
        }   
    }
    
    if(ok)
       printf("\nOutput set verified correctly.\n");
    else
       printf("\nOutput set DID NOT compute correctly!!!\n");

    free(I);   
}

int main(int argc, char* argv[])
{                        
    //clock_t startTime, endTime;
    double timeSeconds;
    int num;

    // Tratando a entrada
    if (argc != 4) 
    {
        printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
        exit(EXIT_FAILURE);
    } 
    else 
    {
        nThreads = atoi(argv[3]);

        if (nThreads < THREAD_MIN || nThreads > THREAD_MAX) 
        {
            printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
            printf("<Quantidade de threads> tem que ser entre %d e %d\n", THREAD_MIN, THREAD_MAX);
            exit(EXIT_FAILURE);
        }  

        n = atoi(argv[1]);
        k = atoi(argv[2]);
        
        if (k < 0 || k > n) 
        {
            printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
            printf("<Quantidade de K menores> tem que ser entre 1 e <Quantidade de números aleatórios>\n");
            exit(EXIT_FAILURE);
        }       
    }

    // Aloca espaços
    pthread_t threads[nThreads];
    input = malloc(n * sizeof(par_t));
    output = malloc(k * sizeof(par_t));
    heaps = malloc(nThreads * sizeof(par_t *));
    tamHeaps = malloc(nThreads * sizeof(int));

    // Randomiza a SEED
    srand(time(NULL));

    // Cria o vetor v
    geraNaleatorios(input, n);

    // Aloca espaço para cada heap
    for (int i = 0; i < nThreads; i++)
    {
        tamHeaps[i] = 0;
        heaps[i] = malloc(k * sizeof(par_t));
    }

    // Printa vetor de aleatórios
    // printf("Vetor de aleatórios:\n");
    // for (int i = 0; i < n; i++)
    //     printf("%0.f ", input[i]);
    // printf("\n");

    // Começa o cronômetro
    chronometer_t time;
    chrono_reset(&time);
    chrono_start(&time);
    
    // Cria as threads
    num = 0;
    for (int i = 0; i < nThreads; i++)
    {
        //pthread_create(&threads[i], NULL, acharKMenores, (void*)num);
        // Testa se criou as threads
        if (pthread_create(&threads[i], NULL, acharKMenores, (void*)num) != 0)
        {
            perror("Erro ao criar threads");
            exit(EXIT_FAILURE);
        }

        num++; 
    }   

    // Aguarda as threads terminarem a execução
    for (int i = 0; i < nThreads; i++)
    {
        //pthread_join(threads[i], NULL);
        // Testa se chegaram ao fim
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Erro ao aguardar as threads\n");
            exit(EXIT_FAILURE);
        }
    }

    // Junta todas as heaps em uma só, montando a solução final
    tamOutput = 0;
    for (int i = 0; i < nThreads; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (tamOutput < k)
                insert(output, &tamOutput, heaps[i][j].chave, heaps[i][j].valor);
            else
                decreaseMax(output, tamOutput, heaps[i][j].chave, heaps[i][j].valor);
        }
    }

    // Para o cronometro
    chrono_stop(&time);

    // Printa K menores
    // printf("\nK menores:\n");
    // for (int i = 0; i < k; i++)
    //     printf("%0.f ", output[i].chave);
    // printf("\n");

    // Printando tempo e MOPS
    chrono_reportTime(&time, "time: ");
    timeSeconds = (double) chrono_gettotal(&time) / ((double)1000 * 1000 * 1000); // NanoSeconds para Seconds
    printf("\nO algoritmo demorou: %lf ms\n", timeSeconds * 1000);  // Milissegundos
    printf("E a vazão foi de: %lf MOPS", (n/timeSeconds));  // MOPS

    // Verifica saída
    verifyOutput(input, output, n, k);

    // Da free em tudo
    for (int i = 0; i < nThreads; i++)
        free(heaps[i]);
    free(heaps);
    free(tamHeaps);

    free(input);
    free(output);

    return 0;
}
