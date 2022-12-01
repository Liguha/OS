int GCD(int a, int b)
{
    while (a != 0)
    {
        b = b % a;
        int tmp = a;
        a = b;
        b = tmp;
    }
    return b;
}

float E(int x)
{
    float a = 1 + 1.0 / x;
    float res = 1.0;
    while (x != 0)
    {
        if (x % 2 == 1)
            res *= a;
        a = a * a;
        x = x / 2;
    }
    return res;
}