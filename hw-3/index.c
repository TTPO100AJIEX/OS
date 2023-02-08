#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

unsigned long long int factorial(unsigned long long int input)
{
    unsigned long long int answer = 1;
    for (unsigned long long int cur = 1; cur <= input; cur++) answer *= cur;
    return answer;
}
unsigned long long int fibonacci(unsigned int index)
{
    unsigned long long int cur = 1, prev = 1;
    for (unsigned int i = 2; i < index; i++)
    {
        unsigned long long int next = prev + cur;
        prev = cur; cur = next;
    }
    return cur;
}

int main(int argc, char** argv)
{
    if (argc != 2) return -1;
    unsigned int input = atoi(argv[1]);

    pid_t chpid = fork();
    if (chpid == -1) return -1;
    if (chpid == 0) printf("factorial by %d is %llu\n", (int)(getpid()), factorial(input));
    else
    {
        printf("fibonacci by %d is %llu\n", (int)(getpid()), fibonacci(input));
        waitpid(chpid, NULL, 0);
    }
    return 0;
}