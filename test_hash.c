#include <stdio.h>
#include "hash.h"

int main(void) {
    char *ssn = "aaaaabbbbbcc";
    printf("%d", hash_ssn(ssn));

}