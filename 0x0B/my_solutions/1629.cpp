#include <iostream>

long long mul(long long A, long long B, long long C)
{
    if(B == 0) return 1;

    long long half = mul(A, B/2, C);
    long long result = (half * half) % C;
    if (B % 2 == 1)
        result = (result * A) % C;
    return result;
}
int main()
{
    long long A, B, C;

    std::cin >> A >> B >> C;

    int answer = mul(A,B,C);
    printf("%lld", answer);

    return 0;
}