#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"

// Inicializa a cache com um tamanho definido pelo utilizador
void initCache(Node *node, int size)
{
    node->cache = (Cache *)malloc(sizeof(Cache));
    if (!node->cache)
    {
        printf("Erro ao alocar memória para cache!\n");
        exit(1);
    }

    Cache *cache = node->cache;
    cache->items = (CacheItem *)malloc(size * sizeof(CacheItem));
    if (!cache->items)
    {
        printf("Erro ao alocar memória para cache items!\n");
        free(node->cache);
        exit(1);
    }

    cache->end = 0;
    cache->maxSize = size;

    for (int i = 0; i < size; i++)
    {
        memset(cache->items[i].name, 0, sizeof(cache->items[i].name));
    }
}
// Adiciona um objeto à cache
void addToCache(Node *node, char *data)
{
    if (!node->cache || !node->cache->items)
    {
        printf("Erro: Cache não inicializada!\n");
        return;
    }

    Cache *cache = node->cache;
    if (cache->end >= cache->maxSize)
    {
        printf("Erro: Índice da cache inválido!\n");
        return;
    }

    memset(cache->items[cache->end].name, 0, sizeof(cache->items[cache->end].name));
    strncpy(cache->items[cache->end].name, data, sizeof(cache->items[cache->end].name) - 1);
    cache->end = (cache->end + 1) % cache->maxSize;
}
// Print da cache
void printCache(Node *node)
{
    printf("Objetos na Cache:\n");
    if (node->cache != NULL)
    {
        for (int i = 0; i < node->cache->maxSize; i++)
        {
            if (strcmp(node->cache->items[i].name, "") != 0)
                printf("%s\n", node->cache->items[i].name);
        }
    }
}
