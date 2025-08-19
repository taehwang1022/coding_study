#include <iostream>
#include <queue>
#include <cmath>
#include <algorithm>


int board[1024][1024];
int vis[1024][1024];
int dist[1024][1024];

using namespace std;
void print_board(int M, int N ,int board[1024][1024])
{
    for(int i =0; i< N; i++)
    {
        for(int j=0; j<M; j++)
        {   
            cout << board[i][j] << " ";
        }
        cout << "\n";
    }
}

int check_board(int M, int N ,int board[1024][1024])
{
    for(int i =0; i< N; i++)
    {
        for(int j=0; j<M; j++)
        {   
            if(board[i][j] == 0)
                return -1;
        }
    }

    return 1;
}

int main()
{
    // ios::sync_with_stdio(0);
    // cin.tie(0);
    
    int M;
    int N;

    int dx[4] = {1, 0, -1, 0};
    int dy[4] = {0, 1, 0, -1};

    queue<pair<int,int>> start_point;

    cin >> M >> N;

    for(int i =0; i< N; i++)
    {
        for(int j=0; j<M; j++)
        {   
            cin >> board[i][j];
            if(board[i][j] == 1)
            {
                start_point.push({i,j});    // 시작지점 잡음
                vis[i][j] = 1;
            }
            
        }
    }

    for(int i =0; i< N; i++)
    {
        for(int j=0; j<M; j++)
        {   
            dist[i][j]=0;     
        }
    }
    int answer =0;


    while(!start_point.empty())
    {
        pair<int,int> cur = start_point.front();
        start_point.pop();
        int x;
        int y;
        for(int i =0; i < 4; i++)
        {
            x = cur.first + dx[i];
            y = cur.second + dy[i];
            if(x <0 || x >= N || y<0 || y >=M)
            {
                //cout << "범위초과 : " << x << " " << y << "\n";
                continue;
            }
            if(board[x][y] == -1 || vis[x][y])
            {
                //cout << "이미방문 : " << x << " " << y << "\n";
                continue;
            }

            dist[x][y] = dist[cur.first][cur.second] + 1;
            vis[x][y] =1 ;
            board[x][y] =1;
            if(answer < dist[x][y])
            {
                answer = dist[x][y];
            }
            //cout << " answer update :" << answer << " x : " << x << " y : " << y << "\n";
            start_point.push({x,y});

        }


    }

    if(check_board(M,N,board) == 1)
    cout <<answer;
    else
        cout << "-1";

    //print_board(M,N,board);



}