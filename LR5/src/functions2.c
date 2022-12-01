int GCD(int a, int b)
{
    int res = 1;
    for (int i = 2; i <= a && i <= b; i++)
    {
        if (a % i == 0 && b % i == 0)
            res = i;
    }
    return res;
}

float E(int x)
{
    float res = 1.0;
    long long n = 1;
    for (int i = 1; i <= x; i++)
    {
        n = n * i;
        res += 1.0 / n;
    }
    return res;
}