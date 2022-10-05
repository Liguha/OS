#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#define MAX_THREADS 12ll
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
    unsigned int seed = 0;
    int er;
    scanf("%lld %u", &rounds, &seed);
    int d = rounds % MAX_THREADS;
    long long data[MAX_THREADS][2];
    pthread_t threads[MAX_THREADS];
    if (er = pthread_mutex_init(&mutex, NULL))
    {
        printf("Mutex init error: %d", er);
        return -1;
    }
    for (int i = 0; i < min(MAX_THREADS, rounds); i++)
    {
        data[i][0] = rounds / MAX_THREADS + (d > 0);
        d--;
        data[i][1] = seed + i;
        if (er = pthread_create(&threads[i], NULL, make_rounds, (void*)data[i]))
        {
            printf("Thread create error: %d", er);
            return -1;
        }
    }
    for (int i = 0; i < min(MAX_THREADS, rounds); i++)
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
    double res = (double)counter / rounds;
    printf("%lf\n", res);
}