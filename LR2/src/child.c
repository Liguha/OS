#include "unistd.h"
#include "stdio.h"
#include "sys/stat.h"
#include "fcntl.h"

int main(int argc, char* argv[])
{
    int p1 = open("pipe1", O_RDONLY);
    int p2 = open("pipe2", O_WRONLY);
    if (p1 == -1 || p2 == -1)
    {
        perror("Child: pipe open error");
        return -1;
    }
    unlink(argv[0]);
    int fout = open(argv[0], O_CREAT | O_WRONLY, S_IREAD);
    if (fout == -1)
    {
        perror("Child: file error");
        return -1;
    }
    if (dup2(p1, 0) == -1 || dup2(fout, 1) == -1)
    {
        perror("Child: dup error");
        return -1;
    }

    char* str;
    size_t n = 0;
    int s = getline(&str, &n, stdin);
    char err[19] = "Last symbol is \'.\'";
    char ok[3] = "01";
    while (s > 0)
    {
        printf("%d\n", s);
        if (str[s - 2] == '.' || str[s - 2] == ';')
        {
            printf("%s", str);
            if (write(p2, &ok[1], 1) == -1)
            {
                perror("Child: write error");
                return -1;
            }
        }
        else
        {
            err[16] = str[s - 2];
            if (write(p2, &ok[0], 1) == -1 || write(p2, err, 19) == -1)
            {
                perror("Child: write error");
                return -1;
            }
        }
        s = getline(&str, &n, stdin);
    } 
    close(p1);
    close(p2);
}