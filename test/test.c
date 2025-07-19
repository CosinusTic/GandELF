#include <stdio.h>

int add(int a, int b)
{
    return a + b;
}

int mult(int a, int b)
{
    return a * b;
}

int divide(int a, int b)
{
    if (b == 0)
        return -1;

    return a / b;
}

int main(void)
{
    int x = 8;
    int y = 2;

    int sum = add(x, y);
    int prod = mult(x, y);
    int quot = divide(x, y);

    printf("x = %d, y = %d\nsum = %d\nprod = %d\nquot = %d\n", x, y, sum, prod,
           quot);

    return 0;
}
