#include <bits/stdc++.h>

int main()
{
    // a = 97
    // z= 122
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    std::string word;
    std::vector<int> answer(26);
    std::cin >> word;

    for(int e : word)
    {
        answer[e-97]++;
    }

    for(int e : answer)
    {
        std::cout << e << " ";
    }


}