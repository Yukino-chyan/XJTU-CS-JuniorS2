#include <stdio.h>
 

extern long fact(long n, long a);
 
int main(void) {
    long n = 5;
    long a = 1;
    long result = fact(n, a);
    printf("fact(%ld, %ld) = %ld\n", n, a, result);
 
    // 验证
    if (result == 120) {
        printf("OK: 5! = 120\n");
    } else {
        printf("FAIL: expected 120, got %ld\n", result);
        return 1;
    }
    return 0;
}
 