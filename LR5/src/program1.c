#include "stdio.h"
#include "functions.h"

#ifdef SYSTEM
    #define PRINT_OS printf("Operation system: %s\n", SYSTEM)
#else
    #define PRINT_OS
#endif

int main()
{
    while (1)
    {
        int t;
        scanf("%d", &t);
        if (t == 1)
        {
            int a, b;
            scanf("%d %d", &a, &b);
            PRINT_OS;
            printf("GCD: %d\n", GCD(a, b));
        }
        if (t == 2)
        {
            int x;
            scanf("%d", &x);
            PRINT_OS;
            printf("E: %.10f\n", E(x));
        }
        if (t == -1)
            break;
    }
}