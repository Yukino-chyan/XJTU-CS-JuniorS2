int add(int a, int b) {
    return a + b;
}

float average(int x, int y) {
    float result = (x + y) / 2.0e0;
    return result;
}

void check(int n) {
    if (n >= 100) {
        print(n);
    } else if (n <= 0) {
        print(0);
    } else {
        int i = 1;
        while (i <= n) {
            i += 1;
        }
        print(i);
    }
}

int main() {
    int a = 3;
    int b = 7;
    int sum = add(a, b);
    float avg = average(a, b);
    int flag = (sum != 0) && (avg >= 1.5);
    if (flag || a > 0) {
        sum += 10;
        b++;
    }
    check(sum);
    return 0;
}
