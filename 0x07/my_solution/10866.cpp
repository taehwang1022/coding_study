#include <bits/stdc++.h>


int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    //명령의 수 N (1 ≤ N ≤ 10,000)
    int N;

    std::deque<int> DQ;


    std::cin >> N;
    std::string order;

    for(int i=0; i< N; i++)
    {
        std::cin >> order;
        int X =0;
        if(order == "push_front")
        {
            std::cin >> X;
            DQ.push_front(X);
        }
        else if(order == "push_back")
        {
            std::cin >> X;
            DQ.push_back(X);
        }
        else if(order == "pop_front")
        {
            if(DQ.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << DQ.front() << "\n";
                DQ.pop_front();
                
            }

        }        
        else if(order == "pop_back")
        {
            if(DQ.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << DQ.back() << "\n";
                DQ.pop_back();
            }
        }
        else if(order == "size")
        {
            X = DQ.size();
            std::cout << X << "\n";
        }
        else if(order == "empty")
        {
            if(DQ.empty())
            {
                std::cout << "1" << "\n";
            }
            else
            {
                std::cout << "0" << "\n";
            }
        }    
        else if(order == "front")
        {
            if(DQ.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << DQ.front() << "\n";
            }
        }   
        else if(order == "back")
        {
            if(DQ.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << DQ.back() << "\n";
            }
        }   



    }


}



