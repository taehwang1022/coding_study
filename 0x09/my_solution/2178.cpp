#include <cmath>
#include <queue>
#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>

using namespace std;

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);
    
    int board[102][102];
    int vis[102][102];
    int dist[102][102];
    int n, m;

    int dx[4] = {1,0,-1,0};
    int dy[4] = {0,1,0,-1};

    cin >> n >> m;

    for(int i =0; i <n; i++)
    {
        string line;
        cin >> line;
        for(int j =0; j < m; j++)
        {
            board[i][j] = line[j] -'0';
        }
    }

    for(int i =0; i <n; i++)
    {
        for(int j =0; j < m; j++)
        {
            dist[i][j] = 0;
        }
    }

    queue<pair<int,int>> Q;
    Q.push({0,0});
    vis[0][0] =1;

    while(!Q.empty())
    {
        
        auto cur = Q.front();
        Q.pop();
        for(int i =0 ; i < 4; i++)
        {
            int nx = cur.first + dx[i];
            int ny = cur.second + dy[i];
            if(nx < 0 || nx >= n || ny<0 || ny>=m)
            {
                continue;
            }
            if(board[nx][ny] != 1 || vis[nx][ny])
            {
                continue;
            }
            vis[nx][ny]= 1;
            dist[nx][ny] = dist[cur.first][cur.second]+1;
            Q.push({nx,ny});
        }


    }
    cout << dist[n-1][m-1] + 1 << '\n'; // 시작점 포함 시 +1    


    

}