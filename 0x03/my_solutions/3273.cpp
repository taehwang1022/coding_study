#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int N;
    int num_box[1000001] ={};
    int add_box[2000001] ={};
    std::cin >> N;

    for(int i =0 ; i< N; i++)
    {
        std::cin >> num_box[i];
        add_box[num_box[i]]++;
    }
    int answer_count=0;
    int answer_num;
    std::cin >> answer_num;

    for(int i =0 ; i< N; i++)
    {
        if((answer_num - num_box[i])>0 && add_box[answer_num - num_box[i]])
        {
            answer_count ++;
        }

    }

    std::cout << answer_count/2;

}


