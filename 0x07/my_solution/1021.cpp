#include <bits/stdc++.h>
void function_1(std::deque<int> &DQ)
{
    DQ.pop_front();
}
void function_2(std::deque<int> &DQ)
{
    int element;
    element = DQ.front();
    DQ.pop_front();
    DQ.push_back(element);


}
void function_3(std::deque<int> &DQ)
{
    int element;
    element = DQ.back();
    DQ.pop_back();
    DQ.push_front(element);

}

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);


    // 첫 번째 원소를 뽑아낸다. 이 연산을 수행하면, 원래 큐의 원소가 a1, ..., ak이었던 것이 a2, ..., ak와 같이 된다.
    // 왼쪽으로 한 칸 이동시킨다. 이 연산을 수행하면, a1, ..., ak가 a2, ..., ak, a1이 된다.
    // 오른쪽으로 한 칸 이동시킨다. 이 연산을 수행하면, a1, ..., ak가 ak, a1, ..., ak-1이 된다.


    int N; // 큐의 크기
    int M; // 추출할 수의 개수

    std::cin >> N; 
    std::cin >> M;
    std::deque<int> DQ;
    int answer=0;

    for(int i=1 ; i < N+1 ; i++)
    {
        DQ.push_back(i);
    }

    while(M--)
    {
        int t;
        std::cin >> t;
        int idx = find(DQ.begin(), DQ.end(),t) - DQ.begin();

        if(DQ.front() == t)
        {
            function_1(DQ);
        }
        else if(idx <= DQ.size()/2)
        {
            while (DQ.front()  != t)
            {
                function_2(DQ);
                answer++;
            }
            function_1(DQ);

        }
        else if(idx > DQ.size()/2)
        {
            while(DQ.front() != t)
            {
                function_3(DQ);
                answer++;
            }
            function_1(DQ);

        }

    }
    std::cout << answer;
    
}

