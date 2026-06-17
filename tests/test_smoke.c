// Smoke test — run via: ./nob test
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    if (1 + 1 != 2)
    {
        fprintf(stderr, "FAIL: basic sanity check\n");
        return 1;
    }

    printf("PASS: smoke test\n");
    return 0;
}
