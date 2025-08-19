#include <bits/stdc++.h>

void parsing(std::deque<int> &tmp_par, std::string tmp);

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);


    // 함수 R은 배열에 있는 수의 순서를 뒤집는 함수이고, 
    // D는 첫 번째 수를 버리는 함수이다.
    // 배열이 비어있는데 D를 사용한 경우에는 에러가 발생한다.
    
    // 첫째 줄에 테스트 케이스의 개수 T가 주어진다. T는 최대 100이다.

    // 각 테스트 케이스의 첫째 줄에는 수행할 함수 p가 주어진다. p의 길이는 1보다 크거나 같고, 100,000보다 작거나 같다.

    // 다음 줄에는 배열에 들어있는 수의 개수 n이 주어진다. (0 ≤ n ≤ 100,000)

    // 다음 줄에는 [x1,...,xn]과 같은 형태로 배열에 들어있는 정수가 주어진다. (1 ≤ xi ≤ 100)

    // 전체 테스트 케이스에 주어지는 p의 길이의 합과 n의 합은 70만을 넘지 않는다.

    int T;
    std::cin >> T;

    while(T--)
    {
        std::string mission;
        int n;
        std::string tmp;
        std::deque<int> tmp_par;
        int reverse=0;

        std::cin >> mission;
        std::cin >> n;
        std::cin >> tmp;
    
        //tmp_par 에 핸들링 해야하는 숫자들 들어있음
        parsing(tmp_par,tmp);
    
        bool error = false;
    
        for(int i=0; i< mission.size();i++)
        {
            if(mission[i] == 'R')
            {
                if(reverse==0)
                    reverse=1;
                else if(reverse==1)
                    reverse=0;
            }
            else if(mission[i] == 'D')
            {
                if(tmp_par.empty())
                {
                    std::cout << "error" << '\n';
                    error = true; 
                    break;
                }
                else if(reverse==0)
                    tmp_par.pop_front();
                else if(reverse==1)
                    tmp_par.pop_back();
            }
        }

        if (error) continue;

        std::cout << '[';
        if (!tmp_par.empty()) 
        {
            if (reverse == 0)
            {
                for (size_t i = 0; i < tmp_par.size(); i++)
                {
                    std::cout << tmp_par[i];
                    if (i < tmp_par.size() - 1) std::cout << ',';
                }
            }
            else if (reverse == 1)
            {
                for (size_t i = tmp_par.size(); i > 0; i--)
                {
                    std::cout << tmp_par[i - 1];
                    if (i > 1) std::cout << ',';
                }
            }
        }
        std::cout << ']' << "\n";
        
    
    }

}

void parsing(std::deque<int> &tmp_par, std::string tmp)
{
    int num=0;
    
    for(int i =0; i<tmp.size() ;i++)
    {
        if(tmp[i] == ',')
        {
            tmp_par.push_back(num);
            num=0;
        }
        else if(tmp[i]>='0' && tmp[i]<='9')
        {
            num = num*10 +(tmp[i]-'0');
        }
    }
    if(num!=0)
    {
        tmp_par.push_back(num);
    }


}