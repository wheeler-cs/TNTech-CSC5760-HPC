/**
 * Using C/C++, add 1E − 18 to the a double variable starting with the value
 * 1.0, 100,000 times. Output the result in high precision. Next, add the number
 * 1E − 18 to a double variable starting with the value 0, also 100,000 times.
 * Then add 1 to that sum, and compare the results in the two variables by
 * subtracting them, and also output them in high precision. What’s the
 * difference in these two sums? (For fun, also output the results of the sums
 * in hexadecimal to see any differences in bit patterns, so you learn how to
 * basically inspect floating point numbers in hex for debugging.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
    // Init variables
    double a, b, c, i;
    a = 1E-18;
    b = 1.0;
    c = 0.0;

    for(i = 0; i < 100000; i++)
    {
        b += a;
    }

    for(i = 0; i < 100000; i++)
    {
        c += a;
    }
    c += 1.0;

    printf("Sum A: %.50lf (%A)\n", b, b);
    printf("Sum B: %.50lf (%A)\n", c, c);
    printf("Difference: %.50lf\n", c - b);

    fflush(stdout);

    return 0;
}