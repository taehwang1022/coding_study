#include <iostream>
#include <cmath>
#include <algorithm>
#include <queue>

using namespace std;

int vis[100002];

int main()
{
    ios::sync_with_stdio(0);
    cin.tie(0);

    bool find =false;

    int N;  //수빈이 위치
    int K;  //동생 위치

    for (int i=0; i<100002; i++)
    {
        vis[i] = -1;
    }
    queue<int> now;

    cin >> N >> K;

    if(N == K)
    {
        cout << "0";
        return 0;
    }
    vis[N] = 1;

    

    now.push(N);

    while(!find)
    {
        int cur = now.front();
        now.pop();

        int dx[3] = {cur +1, cur -1, cur*2};

        for(int i =0 ;i <3; i++)
        {
            if(dx[i]>100000||dx[i]<0)
            {
                continue;
            }
            else if(vis[dx[i]]>0)
            {
                continue;
            }
            else if(dx[i]==K)
            {
                // cout << "찾았다 : " << dx[i]; 
                cout << vis[cur];
                find= true;
                return 0;
            }
            // cout << " 어디로 이동 : " << dx[i] << " 지금 몇턴쨰 : " << vis[cur] <<"\n";
            now.push(dx[i]);
            vis[dx[i]] = vis[cur]+1;
        }

    }
}