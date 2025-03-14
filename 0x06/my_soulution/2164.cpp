#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    std::queue<int> q;

    int N;

    std::cin >> N;

    for(int i =1 ; i< N+1; i++)
    {
        q.push(i);
    }

    while(1)
    {
        if(q.size() == 1)
        {
            break;
        }
        int num;
        q.pop();
        num = q.front();
        q.pop();
        q.push(num);
    }
    std::cout << q.front() << "\n";
}