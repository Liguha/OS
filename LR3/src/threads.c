#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#define DECK_SIZE 52

pthread_mutex_t mutex;
long long counter = 0;

long long min(long long l, long long r)
{
    if (l > r)
        return r;
    return l;
}

void* make_rounds(void* args)
{
    long long n = *(long long*)args;
    unsigned int seed = (unsigned int)*(long long*)(args + sizeof(void*));
    int res = 0;
    for (int i = 0; i < n; i++)
    {
        int a = rand_r(&seed) % DECK_SIZE;
        int b = rand_r(&seed) % (DECK_SIZE - 1);
        b += (b >= a);
        res += (a % (DECK_SIZE / 4)) == (b % (DECK_SIZE / 4));
    }
    long long er;
    if (er = pthread_mutex_lock(&mutex))
        return (void*)er;
    counter += res;
    if (er = pthread_mutex_unlock(&mutex))
        return (void*)er;
    return NULL;
}

int main(int argc, char* argv[])
{  
    long long rounds = 1;
    int max_threads = 0;
    unsigned int seed = time(NULL);
    int er;
    scanf("%d %lld", &max_threads ,&rounds);
    int d = rounds % max_threads;
    long long** data = (long long**)calloc(max_threads, sizeof(long long*));
    if (data == NULL)
    {
        printf("Alloc error");
        return -1;
    }
    for (int i = 0; i < max_threads; i++)
    {
        data[i] = (long long*)calloc(2, sizeof(long long));
        if (data[i] == NULL)
        {
            printf("Alloc error");
            return -1;
        }
    }
    pthread_t* threads = (pthread_t*)calloc(max_threads, sizeof(pthread_t));
    if (threads == NULL)
    {
        printf("Alloc error");
        return -1;
    }
    if (er = pthread_mutex_init(&mutex, NULL))
    {
        printf("Mutex init error: %d", er);
        return -1;
    }
    for (int i = 0; i < min(max_threads, rounds); i++)
    {
        data[i][0] = rounds / max_threads + (d > 0);
        d--;
        data[i][1] = seed + i;
        if (er = pthread_create(&threads[i], NULL, make_rounds, (void*)data[i]))
        {
            printf("Thread create error: %d", er);
            return -1;
        }
    }
    for (int i = 0; i < min(max_threads, rounds); i++)
    {
        long long out = 0;
        if (er = pthread_join(threads[i], (void**)&out))
        {
            printf("Thread join error: %d", er);
            return -1;
        }
        if (out)
        {
            printf("Mutex lock/unlock error: %lld", out);
            return -1;
        }
    }
    if (er = pthread_mutex_destroy(&mutex))
    {
        printf("Mutex destroy error: %d", er);
        return -1;
    }
    free((void*)threads);
    for (int i = 0; i < max_threads; i++)
        free((void*)data[i]);
    free((void*)data);
    double res = (double)counter / rounds;
    printf("%lf\n", res);
}