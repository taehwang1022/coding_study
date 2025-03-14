#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int student[2][6]={};

    int student_num;
    int room_size;
    int answer=0;

    std::cin >> student_num >> room_size;

    for(int i = 0; i < student_num; i++)
    {
        int sex;
        int grade;
        std::cin >> sex >> grade;
        student[sex][grade-1]++;
    }

    for (int s = 0; s < 2; s++)
    {
        for (int g = 0; g < 6; g++)
        {
            if(student[s][g] % room_size > 0)
                answer += (student[s][g] / room_size)+1;
            else
                answer += (student[s][g] / room_size);

        }
    }

    std::cout << answer;
    
}