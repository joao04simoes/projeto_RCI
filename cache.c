#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"

// Inicializa a cache com um tamanho definido pelo utilizador
void initCache(Node *node, int size)
{
    node->cache = (Cache *)malloc(sizeof(Cache));
    if (!node->cache) // Verificar se malloc falhou
    {
        printf("Erro ao alocar memória para cache!\n");
        exit(1);
    }

    Cache *cache = node->cache;
    cache->items = (CacheItem *)malloc(size * sizeof(CacheItem));
    if (!cache->items) // Verificar se malloc falhou
    {
        printf("Erro ao alocar memória para cache items!\n");
        free(node->cache); // Evita memory leak
        exit(1);
    }

    cache->end = 0;
    cache->maxSize = size;

    // Inicializar os nomes com strings vazias
    for (int i = 0; i < size; i++)
    {
        memset(cache->items[i].name, 0, sizeof(cache->items[i].name));
    }
}

void addToCache(Node *node, char *data)
{
    if (!node->cache || !node->cache->items) // Verificar se cache foi inicializada
    {
        printf("Erro: Cache não inicializada!\n");
        return;
    }

    printf("antes de iniciar\n");
    Cache *cache = node->cache;
    if (cache->end >= cache->maxSize) // Garantir que não acessamos fora do array
    {
        printf("Erro: Índice da cache inválido!\n");
        return;
    }
    printf("antes do memset\n");
    memset(cache->items[cache->end].name, 0, sizeof(cache->items[cache->end].name));
    printf("antes do copy\n");
    strncpy(cache->items[cache->end].name, data, sizeof(cache->items[cache->end].name) - 1);
    printf("antes de atualiar o end\n");
    cache->end = (cache->end + 1) % cache->maxSize;
}

void printCache(Node *node)
{

    printf("Objetos na Cache:\n");
    if (node->cache != NULL)
    {
        for (int i = 0; i < node->cache->maxSize; i++)
        {
            printf("%s\n", node->cache->items[i].name);
        }
    }
}
