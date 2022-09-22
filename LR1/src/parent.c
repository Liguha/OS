#include "unistd.h"
#include "stdio.h"
#include "sys/stat.h"
#include "fcntl.h"

int main(int argc, char* argv[])
{
    unlink("pipe1");
    unlink("pipe2");
    if (mkfifo("pipe1", S_IREAD | S_IWRITE) == -1 || mkfifo("pipe2", S_IREAD | S_IWRITE) == -1)
    {
        perror("Parent: pipe create error");
        return -1;
    }
    char* fout;
    size_t k = 0;
    if (getline(&fout, &k, stdin) <= 0)
    {
        perror("Parent: file name error");
        return -1;
    }

    int id = fork();
    if (id == -1)
    {
        perror("Parent: fork error");
        return -1;
    }
    if (id == 0)
    {
        int p1 = open("pipe1", O_WRONLY);
        int p2 = open("pipe2", O_RDONLY);
        if (p1 == -1 || p2 == -1)
        {
            perror("Parent: pipe open error");
            return -1;
        }
        char* str;
        size_t n = 0;
        int s = getline(&str, &n, stdin);
        while (s > 0)
        {
            if (write(p1, str, s) == -1)
            {
                perror("Parent: write error");
                return -1;
            }
            char ok;
            if (read(p2, &ok, 1) <= 0)
            {
                perror("Parent: read error");
                return -1;
            }
            if (ok == '0')
            {
                char ans[19];
                if (read(p2, ans, 19) <= 0)
                {
                    perror("Parent: read error");
                    return -1;
                }
                printf("%s\n", ans);
            }
            s = getline(&str, &n, stdin);
        }        
        close(p1);
        close(p2);
        unlink("pipe1");
        unlink("pipe2");
    }
    else
    {
        if (execl("child.out", fout, NULL) == -1)
        {
            perror("Child: exec error");
            return -1;
        }
    }
}