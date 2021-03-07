#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <queue>
#include "jsoncpp/json.h"

using namespace std;

const int none = 0, brick = 1, forest = 2, steel = 4, water = 8;
const int px[4] = {0, 1, 0, -1}, py[4] = {-1, 0, 1, 0};

int state[10][10] = {0}, pre_state[10][10] = {0};
int enemy_position[4] = {0}, self_position[4] = {0};
int pre_enemy_position[4] = {0};
int myside;
int value[2][9] = {0};
int pre_enemy_action[2] = {0}, pre_self_action[2] = {0};
int action[2] = {0};
int shoot_cnt[2] = {0};
int enemyenterforest[4] = {-3, -3, -3, -3}, interval[2] = {0};
int stayinterval[2] = {0};

bool ok(int x, int y) {
    return x >= 0 && x <= 8 && y >= 0 && y <= 8 && (~state[x][y] & steel) && (~state[x][y] & water) &&
    (~state[x][y] & brick);
}

bool valid(int id, int action) {
    if (action == 8)action = -1;
    if (action >= 4 && shoot_cnt[id])return false;
    if (action == -1 || action >= 4)return true;
    int xx = self_position[id * 2] + px[action];
    int yy = self_position[id * 2 + 1] + py[action];
    if (!ok(xx, yy))return false;
    for (int i = 0; i < 2; i++) {
        if (enemy_position[i * 2] >= 0 && state[enemy_position[i * 2]][enemy_position[i * 2 + 1]] == 0) {
            if ((xx - enemy_position[i * 2] == 0) && (yy - enemy_position[i * 2 + 1] == 0))
                return false;
        }
        if (self_position[i * 2] >= 0) {
            if ((xx - self_position[i * 2] == 0) && (yy - self_position[i * 2 + 1] == 0))
                return false;
        }
    }
    return true;
}

const int plusvalue = 200;
int steps[10][10];
int prex[10][10], prey[10][10];
bool visited[10][10];
int x_tochoose, y_tochoose;

struct point_info {
    int x;
    int y;
    int steps;
    
    point_info(int X, int Y, int STEPS) : x(X), y(Y), steps(STEPS) {}
    
    bool operator<(const point_info &info) const {
        if (steps > info.steps)return 1;
        else return 0;
    }
};

void pre_findroute(int x0, int y0) {
    x_tochoose = x0;
    y_tochoose = y0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            steps[i][j] = 10000;
            visited[i][j] = 0;
            prex[i][j] = x0;
            prey[i][j] = y0;
        }
    }
    if (x0 != -1) {
        steps[x0][y0] = 0;
    }
}

void markroute(int x0, int y0) {
    
    priority_queue<point_info> explore;
    point_info p0(x0, y0, 0);
    explore.push(p0);
    while (!explore.empty()) {
        bool flag = 0;
        while (visited[explore.top().x][explore.top().y] == 1) {
            explore.pop();
            if (explore.empty()) {
                flag = 1;
                break;
            }
        }
        if (flag == 1)
            break;
        int nowx = explore.top().x;
        int nowy = explore.top().y;
        int nowstep = explore.top().steps;
        visited[nowx][nowy] = 1;
        explore.pop();
        for (int i = 0; i < 4; i++) {
            if (nowx + px[i] >= 0 && nowx + px[i] < 9 && nowy + py[i] >= 0 && nowy + py[i] < 9 &&
                visited[nowx + px[i]][nowy + py[i]] == 0 &&
                !(((myside == 0) && (nowx + px[i] == 4) && (nowy + py[i] == 0)) ||
                  ((myside == 1) && (nowx + px[i] == 4) && (nowy + py[i] == 8)))) {
                    if (state[nowx + px[i]][nowy + py[i]] == none) {
                        if (nowstep + 1 < steps[nowx + px[i]][nowy + py[i]]) {
                            steps[nowx + px[i]][nowy + py[i]] = nowstep + 1;
                            prex[nowx + px[i]][nowy + py[i]] = nowx;
                            prey[nowx + px[i]][nowy + py[i]] = nowy;
                        }
                        point_info temp(nowx + px[i], nowy + py[i], nowstep + 1);
                        explore.push(temp);
                    }
                    if (state[nowx + px[i]][nowy + py[i]] == brick) {
                        if (nowstep + 2 < steps[nowx + px[i]][nowy + py[i]]) {
                            steps[nowx + px[i]][nowy + py[i]] = nowstep + 2;
                            prex[nowx + px[i]][nowy + py[i]] = nowx;
                            prey[nowx + px[i]][nowy + py[i]] = nowy;
                        }
                        point_info temp(nowx + px[i], nowy + py[i], nowstep + 2);
                        explore.push(temp);
                    }
                    if (state[nowx + px[i]][nowy + py[i]] == forest) {
                        if (nowstep + 1 < steps[nowx + px[i]][nowy + py[i]]) {
                            steps[nowx + px[i]][nowy + py[i]] = nowstep + 1;
                            prex[nowx + px[i]][nowy + py[i]] = nowx;
                            prey[nowx + px[i]][nowy + py[i]] = nowy;
                        }
                        point_info temp(nowx + px[i], nowy + py[i], nowstep + 1);
                        explore.push(temp);
                    }
                }
        }
    }
}

int DESX, DESY;

void choose_des(int x0, int y0) {
    DESX = x0;
    DESY = y0;
    pre_findroute(x0, y0);
    markroute(x0, y0);
    int desx, desy;
    int minround;
    if (myside == 0) {
        desx = 4;
        desy = 8;
        minround = steps[desx][desy];
        int bricknum = 0;
        for (int i = 4; i >= 0 && state[i][8] != steel; i--) {
            if (state[i][8] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][8] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 8;
                minround = steps[i][8] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 4; i <= 8 && state[i][8] != steel; i++) {
            if (state[i][8] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][8] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 8;
                minround = steps[i][8] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 8; i >= 0 && state[4][i] != steel; i--) {
            if (state[4][i] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[4][i] + 2 * bricknum + 1 <= minround) {
                desx = 4;
                desy = i;
                minround = steps[4][i] + 2 * bricknum + 1;
            }
        }
    } else {
        desx = 4;
        desy = 0;
        minround = steps[desx][desy];
        int bricknum = 0;
        for (int i = 4; i >= 0 && state[i][0] != steel; i--) {
            if (state[i][0] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][0] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 0;
                minround = steps[i][0] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 4; i <= 8 && state[i][0] != steel; i++) {
            if (state[i][0] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][0] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 0;
                minround = steps[i][0] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 0; i <= 8 && state[4][i] != steel; i++) {
            if (state[4][i] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[4][i] + 2 * bricknum + 1 <= minround) {
                desx = 4;
                desy = i;
                minround = steps[4][i] + 2 * bricknum + 1;
            }
        }
    }
    DESX = desx;
    DESY = desy;
    //cout<<"des:("<<desx<<","<<desy<<")"<<endl;
}

int E_DESX, E_DESY;

void enemy_choose_des(int x0, int y0) {
    DESX = x0;
    DESY = y0;
    pre_findroute(x0, y0);
    markroute(x0, y0);
    int desx, desy;
    int minround;
    if (myside == 1) {
        desx = 4;
        desy = 8;
        minround = steps[desx][desy];
        int bricknum = 0;
        for (int i = 4; i >= 0 && state[i][8] != steel; i--) {
            if (state[i][8] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][8] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 8;
                minround = steps[i][8] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 4; i <= 8 && state[i][8] != steel; i++) {
            if (state[i][8] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][8] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 8;
                minround = steps[i][8] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 8; i >= 0 && state[4][i] != steel; i--) {
            if (state[4][i] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[4][i] + 2 * bricknum + 1 <= minround) {
                desx = 4;
                desy = i;
                minround = steps[4][i] + 2 * bricknum + 1;
            }
        }
    } else {
        desx = 4;
        desy = 0;
        minround = steps[desx][desy];
        int bricknum = 0;
        for (int i = 4; i >= 0 && state[i][0] != steel; i--) {
            if (state[i][0] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][0] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 0;
                minround = steps[i][0] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 4; i <= 8 && state[i][0] != steel; i++) {
            if (state[i][0] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[i][0] + 2 * bricknum + 1 <= minround) {
                desx = i;
                desy = 0;
                minround = steps[i][0] + 2 * bricknum + 1;
            }
        }
        bricknum = 0;
        for (int i = 0; i <= 8 && state[4][i] != steel; i++) {
            if (state[4][i] == brick)
                bricknum++;
            //这里要商量一下有没有等于
            if (steps[4][i] + 2 * bricknum + 1 <= minround) {
                desx = 4;
                desy = i;
                minround = steps[4][i] + 2 * bricknum + 1;
            }
        }
    }
    E_DESX = desx;
    E_DESY = desy;
    //cout<<"des:("<<desx<<","<<desy<<")"<<endl;
}

int NOT_GOOD = 0;

void findroute(int x0, int y0) {
    NOT_GOOD = 0;
    if (x0 == -1)return;
    pre_findroute(x0, y0);
    markroute(x0, y0);
    choose_des(x0, y0);
    int nowx = DESX, nowy = DESY;
    while (true) {
        if (myside == 0 && nowx == 3 && nowy == 0)NOT_GOOD = 1;
        if (myside == 0 && nowx == 5 && nowy == 0)NOT_GOOD = 2;
        if (myside == 1 && nowx == 3 && nowy == 8)NOT_GOOD = 1;
        if (myside == 1 && nowx == 5 && nowy == 8)NOT_GOOD = 2;
        if (nowx == x0 && nowy == y0)
            break;
        x_tochoose = nowx;
        y_tochoose = nowy;
        int temp = nowx;
        nowx = prex[nowx][nowy];
        nowy = prey[temp][nowy];
    }
}

void enemy_findroute(int x0, int y0) {
    if (x0 == -1)return;
    pre_findroute(x0, y0);
    markroute(x0, y0);
    enemy_choose_des(x0, y0);
    int nowx = E_DESX, nowy = E_DESY;
    while (true) {
        if (nowx == x0 && nowy == y0)
            break;
        x_tochoose = nowx;
        y_tochoose = nowy;
        int temp = nowx;
        nowx = prex[nowx][nowy];
        nowy = prey[temp][nowy];
    }
}

const int stepgap = 1;

void changeroute(int tanknum) {
    if (NOT_GOOD == 0)
        return;
    else {
        int tempminstep =steps[DESX][DESY];
        int holdinfo = 0;
        int LAST_NOT_GOOD = NOT_GOOD;
        if (myside == 0 && NOT_GOOD == 1) {
            holdinfo = state[3][0];
            state[3][0] = steel;
        }
        if (myside == 0 && NOT_GOOD == 2) {
            holdinfo = state[5][0];
            state[5][0] = steel;
        }
        if (myside == 1 && NOT_GOOD == 1) {
            holdinfo = state[3][8];
            state[3][8] = steel;
        }
        if (myside == 1 && NOT_GOOD == 2) {
            holdinfo = state[5][8];
            state[5][8] = steel;
        }
        findroute(self_position[2 * tanknum], self_position[1 + 2 * tanknum]);
        int tempstep = steps[DESX][DESY];
        if (tempstep - tempminstep > stepgap) {
            if (myside == 0 && LAST_NOT_GOOD == 1) {
                state[3][0] = holdinfo;
                findroute(self_position[2 * tanknum], self_position[1 + 2 * tanknum]);
            }
            if (myside == 0 && LAST_NOT_GOOD == 2) {
                state[5][0] = holdinfo;
                findroute(self_position[2 * tanknum], self_position[1 + 2 * tanknum]);
            }
            if (myside == 1 && LAST_NOT_GOOD == 1) {
                state[3][0] = holdinfo;
                findroute(self_position[2 * tanknum], self_position[1 + 2 * tanknum]);
            }
            if (myside == 1 && LAST_NOT_GOOD == 2) {
                state[5][8] = holdinfo;
                findroute(self_position[2 * tanknum], self_position[1 + 2 * tanknum]);
            }
        }
    }
}

void shortest_route(int x0, int y0, int tanknum) {
    findroute(x0, y0);
    changeroute(tanknum);
    int PX = x_tochoose - x0;
    int PY = y_tochoose - y0;
    /*int px[4] = {0, 1, 0, -1};
     int py[4] = {-1, 0, 1, 0};*/
    //const int none = 0, brick = 1, forest = 2, steel = 4, water = 8;
    if (tanknum == 0) {
        if (PX == 0 && PY == -1) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[0][0] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[0][4] += plusvalue;
            }
        }
        if (PX == 1 && PY == 0) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[0][1] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[0][5] += plusvalue;
            }
        }
        if (PX == 0 && PY == 1) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[0][2] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[0][6] += plusvalue;
            }
        }
        if (PX == -1 && PY == 0) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[0][3] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[0][7] += plusvalue;
            }
        }
    } else {
        if (PX == 0 && PY == -1) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[1][0] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[1][4] += plusvalue;
            }
        }
        if (PX == 1 && PY == 0) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[1][1] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[1][5] += plusvalue;
            }
        }
        if (PX == 0 && PY == 1) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[1][2] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[1][6] += plusvalue;
            }
        }
        if (PX == -1 && PY == 0) {
            if (state[x_tochoose][y_tochoose] == 0 || state[x_tochoose][y_tochoose] == 2) {
                value[1][3] += plusvalue;
            }
            if (state[x_tochoose][y_tochoose] == 1) {
                value[1][7] += plusvalue;
            }
        }
    }
}


struct forest_point {
    int x;
    int y;
    int dis;
    
    forest_point(int X, int Y) {
        x = X;
        y = Y;
        if (myside == 0)
            dis = pow(X - 4, 2) + pow(Y, 2);
        else
            dis = pow(X - 4, 2) + pow(8 - Y, 2);
    }
    
    bool operator<(const forest_point &X) { return dis > X.dis; }
};

bool operator<(const forest_point &X, const forest_point &Y) { return X.dis > Y.dis; }

double forestmap[10][10] = {0};

const double prob = 0.2;

/*double threshold = 0.8;
 double threshold2 = 0;
 double threshold3 = 0.6;*/

double threshold[10][10][3]={0};
struct pointxy{
    int x;
    int y;
    pointxy(int X,int Y){x=X;y=Y;}
};
void choose_threshold(){
    for(int i=0;i<10;i++)
        for(int j=0;j<10;j++)
            for(int k=0;k<3;k++)
                threshold[i][j][k]=0;
    queue<pointxy>tempq1;
    queue<pointxy>tempq2;
    queue<pointxy>tempq3;
    queue<pointxy>tempq4;
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            if(state[i][j]==forest){
                pointxy temp(i,j);
                tempq1.push(temp);
            }
        }
    }
    for(int i=5;i<9;i++){
        for(int j=0;j<4;j++){
            if(state[i][j]==forest){
                pointxy temp(i,j);
                tempq2.push(temp);
            }
        }
    }
    for(int i=0;i<4;i++){
        for(int j=5;j<9;j++){
            if(state[i][j]==forest){
                pointxy temp(i,j);
                tempq3.push(temp);
            }
        }
    }
    for(int i=5;i<9;i++){
        for(int j=5;j<9;j++){
            if(state[i][j]==forest){
                pointxy temp(i,j);
                tempq4.push(temp);
            }
        }
    }
    int pointnum=tempq1.size(),temp_pointnum=pointnum;
    double temp_max=0;
    if(pointnum!=0){
        while(temp_pointnum--){
            int tempX=tempq1.front().x;
            int tempY=tempq1.front().y;
            if(forestmap[tempX][tempY]>temp_max)
                temp_max=forestmap[tempX][tempY];
            tempq1.pop();
            pointxy back(tempX,tempY);
            tempq1.push(back);
        }
        while(!tempq1.empty()){
            int tempX=tempq1.front().x;
            int tempY=tempq1.front().y;
            threshold[tempX][tempY][0]=(double)temp_max/2<0.01?0:(double)temp_max/2;
            threshold[tempX][tempY][2]=(double)temp_max/2-0.2<0.01?0:(double)temp_max/2-0.2;
            tempq1.pop();
        }
    }
    pointnum=tempq2.size();
    temp_pointnum=pointnum;
    temp_max=0;
    if(pointnum!=0){
        while(temp_pointnum--){
            int tempX=tempq2.front().x;
            int tempY=tempq2.front().y;
            if(forestmap[tempX][tempY]>temp_max)
                temp_max=forestmap[tempX][tempY];
            tempq2.pop();
            pointxy back(tempX,tempY);
            tempq2.push(back);
        }
        while(!tempq2.empty()){
            int tempX=tempq2.front().x;
            int tempY=tempq2.front().y;
            threshold[tempX][tempY][0]=(double)temp_max/2<0.01?0:(double)temp_max/2;
            threshold[tempX][tempY][2]=(double)temp_max/2-0.2<0.01?0:(double)temp_max/2-0.2;
            tempq2.pop();
        }
    }
    pointnum=tempq3.size();
    temp_pointnum=pointnum;
    temp_max=0;
    if(pointnum!=0){
        while(temp_pointnum--){
            int tempX=tempq3.front().x;
            int tempY=tempq3.front().y;
            if(forestmap[tempX][tempY]>temp_max)
                temp_max=forestmap[tempX][tempY];
            tempq3.pop();
            pointxy back(tempX,tempY);
            tempq3.push(back);
        }
        while(!tempq3.empty()){
            int tempX=tempq3.front().x;
            int tempY=tempq3.front().y;
            threshold[tempX][tempY][0]=(double)temp_max/2<0.01?0:(double)temp_max/2;
            threshold[tempX][tempY][2]=(double)temp_max/2-0.2<0.01?0:(double)temp_max/2-0.2;
            tempq3.pop();
        }
    }
    pointnum=tempq4.size();
    temp_pointnum=pointnum;
    temp_max=0;
    if(pointnum!=0){
        while(temp_pointnum--){
            int tempX=tempq4.front().x;
            int tempY=tempq4.front().y;
            if(forestmap[tempX][tempY]>temp_max)
                temp_max=forestmap[tempX][tempY];
            tempq4.pop();
            pointxy back(tempX,tempY);
            tempq4.push(back);
        }
        while(!tempq4.empty()){
            int tempX=tempq4.front().x;
            int tempY=tempq4.front().y;
            threshold[tempX][tempY][0]=(double)temp_max/2<0.01?0:(double)temp_max/2;
            threshold[tempX][tempY][2]=(double)temp_max/2-0.2<0.01?0:(double)temp_max/2-0.2;
            tempq4.pop();
        }
    }
};
/*void choose_threshold() {
 double max_guessvalue = 0;
 double sec_max_guessvalue = 0;
 for (int i = 0; i < 10; i++) {
 for (int j = 0; j < 10; j++) {
 if (forestmap[i][j] > max_guessvalue) {
 sec_max_guessvalue = max_guessvalue;
 max_guessvalue = forestmap[i][j];
 } else if (forestmap[i][j] > sec_max_guessvalue)
 sec_max_guessvalue = forestmap[i][j];
 }
 }
 threshold = (double) sec_max_guessvalue / 2 < 0.01 ? 0 : (double) sec_max_guessvalue / 2;
 threshold2 = 0;
 threshold3 = (double) sec_max_guessvalue / 2 - 0.2 < 0.01 ? 0 : (double) sec_max_guessvalue / 2 - 0.2;
 }
 */
void guess_enemy_route(int x0, int y0, int tanknum) {
    if (x0 == -1)return;
    pre_findroute(x0, y0);
    markroute(x0, y0);
    enemy_choose_des(x0, y0);
    int nowx = E_DESX, nowy = E_DESY;
    while (true) {
        if (forestmap[nowx][nowy] > 0 && steps[nowx][nowy] <= interval[tanknum] + 1)
            forestmap[nowx][nowy] += steps[nowx][nowy] >= (interval[tanknum] + 1) / 2 ? (double) 2 * steps[nowx][nowy] /
            (interval[tanknum] + 1) : 0;
        if (nowx == x0 && nowy == y0)
            break;
        int temp = nowx;
        nowx = prex[nowx][nowy];
        nowy = prey[temp][nowy];
    }
}

void guess_enemy() {
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j)forestmap[i][j] = 0;
    }
    int tempinterval[2] = {interval[0], interval[1]};
    if (enemyenterforest[0] != -3 && enemy_position[0] != -1) {
        queue<forest_point> forestq;
        queue<forest_point> old_forestq;
        priority_queue<forest_point> expand_direc;
        for (int i = 0; i < 4; i++) {
            int tempx = enemyenterforest[0] + px[i], tempy = enemyenterforest[1] + py[i];
            if (tempx < 0 || tempx > 8 || tempy < 0 || tempy > 8)continue;
            if (state[tempx][tempy] == forest) {
                forest_point tryto(tempx, tempy);
                expand_direc.push(tryto);
            }
        }
        bool EXPAND[4] = {0};
        int TEMPMINDIS = expand_direc.top().dis;
        while (!expand_direc.empty()) {
            if (expand_direc.top().dis != TEMPMINDIS)
                break;
            if (expand_direc.top().x - enemyenterforest[0] == 0 && expand_direc.top().y - enemyenterforest[1] == -1)
                EXPAND[0] = 1;
            else if (expand_direc.top().x - enemyenterforest[0] == 1 && expand_direc.top().y - enemyenterforest[1] == 0)
                EXPAND[1] = 1;
            else if (expand_direc.top().x - enemyenterforest[0] == 0 && expand_direc.top().y - enemyenterforest[1] == 1)
                EXPAND[2] = 1;
            else if (expand_direc.top().x - enemyenterforest[0] == -1 &&
                     expand_direc.top().y - enemyenterforest[1] == 0)
                EXPAND[3] = 1;
            expand_direc.pop();
        }
        for (int i = 0; i < 4; i++) {
            if (!EXPAND[i])continue;
            int tempx = enemyenterforest[0] + px[i], tempy = enemyenterforest[1] + py[i];
            if (tempx < 0 || tempx > 8 || tempy < 0 || tempy > 8)continue;
            if (state[tempx][tempy] == forest) {
                forestmap[tempx][tempy] = 1 + prob;
                forest_point init(tempx, tempy);
                forestq.push(init);
            }
        }
        while (tempinterval[0]--) {
            int times = forestq.size();
            while (times--) {
                int X = forestq.front().x;
                int Y = forestq.front().y;
                for (int k = 0; k < 4; k++) {
                    if (X + px[k] >= 0 && X + px[k] < 9 && Y + py[k] >= 0 && Y + py[k] < 9 &&
                        state[X + px[k]][Y + py[k]] == forest) {
                        if (forestmap[X + px[k]][Y + py[k]] == 0) {
                            forestmap[X + px[k]][Y + py[k]] = 1;
                            forest_point temp(X + px[k], Y + py[k]);
                            if (temp.dis < forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] += prob;
                            if (temp.dis > forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] -= prob;
                            forestq.push(temp);
                        } else if (forestmap[X][Y] == 1 + prob) {
                            forest_point temp(X + px[k], Y + py[k]);
                            if (temp.dis < forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] = 1 + prob;
                            if (temp.dis > forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] = 1 - prob;
                        }
                    }
                }
                forest_point old(X, Y);
                old_forestq.push(old);
                forestq.pop();
            }
            int oldpoint_num = old_forestq.size();
            while (oldpoint_num--) {
                int tempx = old_forestq.front().x;
                int tempy = old_forestq.front().y;
                forestmap[tempx][tempy] =
                forestmap[tempx][tempy] - prob <= 0.01 ? forestmap[tempx][tempy] : forestmap[tempx][tempy] -
                prob;
                old_forestq.pop();
                forest_point temp(tempx, tempy);
                old_forestq.push(temp);
            }
        }
        //guess_enemy_route(enemyenterforest[0], enemyenterforest[1], 0);
    }
    if (enemyenterforest[2] != -3 && enemy_position[2] != -1) {
        queue<forest_point> forestq;
        queue<forest_point> old_forestq;
        priority_queue<forest_point> expand_direc;
        for (int i = 0; i < 4; i++) {
            int tempx = enemyenterforest[2] + px[i], tempy = enemyenterforest[3] + py[i];
            if (tempx < 0 || tempx > 8 || tempy < 0 || tempy > 8)continue;
            if (state[tempx][tempy] == forest) {
                forest_point tryto(tempx, tempy);
                expand_direc.push(tryto);
            }
        }
        bool EXPAND[4] = {0};
        int TEMPMINDIS = expand_direc.top().dis;
        while (!expand_direc.empty()) {
            if (expand_direc.top().dis != TEMPMINDIS)
                break;
            if (expand_direc.top().x - enemyenterforest[2] == 0 && expand_direc.top().y - enemyenterforest[3] == -1)
                EXPAND[0] = 1;
            else if (expand_direc.top().x - enemyenterforest[2] == 1 && expand_direc.top().y - enemyenterforest[3] == 0)
                EXPAND[1] = 1;
            else if (expand_direc.top().x - enemyenterforest[2] == 0 && expand_direc.top().y - enemyenterforest[3] == 1)
                EXPAND[2] = 1;
            else if (expand_direc.top().x - enemyenterforest[2] == -1 &&
                     expand_direc.top().y - enemyenterforest[3] == 0)
                EXPAND[3] = 1;
            expand_direc.pop();
        }
        for (int i = 0; i < 4; i++) {
            if (!EXPAND[i])
                continue;
            int tempx = enemyenterforest[2] + px[i], tempy = enemyenterforest[3] + py[i];
            if (tempx < 0 || tempx > 8 || tempy < 0 || tempy > 8)continue;
            if (state[tempx][tempy] == forest) {
                forestmap[tempx][tempy] = 1 + prob;
                forest_point init(tempx, tempy);
                forestq.push(init);
            }
        }
        while (tempinterval[1]--) {
            int times = forestq.size();
            while (times--) {
                int X = forestq.front().x;
                int Y = forestq.front().y;
                for (int k = 0; k < 4; k++) {
                    if (X + px[k] >= 0 && X + px[k] < 9 && Y + py[k] >= 0 && Y + py[k] < 9 &&
                        state[X + px[k]][Y + py[k]] == forest) {
                        if (forestmap[X + px[k]][Y + py[k]] == 0) {
                            forestmap[X + px[k]][Y + py[k]] = 1;
                            forest_point temp(X + px[k], Y + py[k]);
                            if (temp.dis < forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] += prob;
                            if (temp.dis > forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] -= prob;
                            forestq.push(temp);
                        } else if (forestmap[X][Y] == 1 + prob) {
                            forest_point temp(X + px[k], Y + py[k]);
                            if (temp.dis < forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] = 1 + prob;
                            if (temp.dis > forestq.front().dis)
                                forestmap[X + px[k]][Y + py[k]] = 1 - prob;
                        }
                    }
                }
                forest_point old(X, Y);
                old_forestq.push(old);
                forestq.pop();
            }
            int oldpoint_num = old_forestq.size();
            while (oldpoint_num--) {
                int tempx = old_forestq.front().x;
                int tempy = old_forestq.front().y;
                forestmap[tempx][tempy] =
                forestmap[tempx][tempy] - prob <= 0.01 ? forestmap[tempx][tempy] : forestmap[tempx][tempy] -
                prob;
                old_forestq.pop();
                forest_point temp(tempx, tempy);
                old_forestq.push(temp);
            }
        }
        //guess_enemy_route(enemyenterforest[2], enemyenterforest[3], 1);
    }
    choose_threshold();
}

int EvaluateGrid(int x, int y) {
    if (myside == 0) {
        if (y <= 4)return 0;
        else return 100 - 10 * (abs(x - 4) + 8 - y);
    } else {
        if (y >= 4)return 0;
        else return 100 - 10 * (abs(x - 4) + y);
    }
}

void diagonal() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (value[i][j] != 0) {
                bool flag = 0;
                if (j == 0 || j == 2) {
                    if (enemy_position[1] == self_position[i * 2 + 1] + py[j]) {
                        if (enemy_position[0] < self_position[i * 2]) {
                            for (int k = enemy_position[0]; k <= self_position[i * 2]; k++) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][enemy_position[1]] == brick ||
                                    state[k][enemy_position[1]] == steel)
                                    break;
                            }
                        } else if (enemy_position[0] > self_position[i * 2]) {
                            for (int k = enemy_position[0]; k >= self_position[i * 2]; k--) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][enemy_position[1]] == brick ||
                                    state[k][enemy_position[1]] == steel)
                                    break;
                            }
                        }
                        if (pre_enemy_action[0] >= 4)
                            flag = 0;
                    }
                    if (enemy_position[3] == self_position[i * 2 + 1] + py[j]) {
                        if (enemy_position[2] < self_position[i * 2]) {
                            for (int k = enemy_position[2]; k <= self_position[i * 2]; k++) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][enemy_position[3]] == brick ||
                                    state[k][enemy_position[3]] == steel)
                                    break;
                            }
                        } else if (enemy_position[2] > self_position[i * 2]) {
                            for (int k = enemy_position[2]; k >= self_position[i * 2]; k--) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][enemy_position[3]] == brick || state[k][enemy_position[3]] == steel)
                                    break;
                            }
                        }
                        if (pre_enemy_action[1] >= 4)
                            flag = 0;
                    }
                } else {
                    if (enemy_position[0] == self_position[i * 2] + px[j]) {
                        if (enemy_position[1] < self_position[i * 2 + 1]) {
                            for (int k = enemy_position[1]; k <= self_position[i * 2 + 1]; k++) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[enemy_position[0]][k] == brick ||
                                    state[enemy_position[0]][k] == steel)
                                    break;
                            }
                        } else if (enemy_position[1] > self_position[i * 2 + 1]) {
                            for (int k = enemy_position[1]; k >= self_position[i * 2 + 1]; k--) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[enemy_position[0]][k] == brick ||
                                    state[enemy_position[0]][k] == steel)
                                    break;
                            }
                        }
                        if (pre_enemy_action[0] >= 4)
                            flag = 0;
                    }
                    if (enemy_position[2] == self_position[i * 2] + px[j]) {
                        if (enemy_position[3] < self_position[i * 2 + 1]) {
                            for (int k = enemy_position[3]; k <= self_position[i * 2 + 1]; k++) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[enemy_position[2]][k] == brick ||
                                    state[enemy_position[2]][k] == steel)
                                    break;
                            }
                        } else if (enemy_position[3] > self_position[i * 2 + 1]) {
                            for (int k = enemy_position[3]; k >= self_position[i * 2 + 1]; k--) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[enemy_position[2]][k] == brick ||
                                    state[enemy_position[2]][k] == steel)
                                    break;
                            }
                        }
                        if (pre_enemy_action[1] >= 4)
                            flag = 0;
                    }
                }
                if (flag == 1) {
                    value[i][j] -= 1000;
                }
            }
        }
    }
}

void guess_diagonal() {
    guess_enemy();
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (value[i][j] != 0) {
                bool flag = 0;
                if (j == 0 || j == 2) {
                    for (int m = self_position[i * 2] - 1; m >= 0; m--) {
                        if (forestmap[m][self_position[i * 2 + 1] + py[j]] > threshold[m][self_position[i * 2 + 1] + py[j]][0]) {
                            for (int k = m; k <= self_position[i * 2]; k++) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][self_position[i * 2 + 1] + py[j]] == brick ||
                                    state[k][self_position[i * 2 + 1] + py[j]] == steel)
                                    break;
                            }
                            break;
                        }
                    }
                    for (int m = self_position[i * 2] + 1; m < 9; m++) {
                        if (forestmap[m][self_position[i * 2 + 1] + py[j]] > threshold[m][self_position[i * 2 + 1] + py[j]][0]) {
                            for (int k = m; k >= self_position[i * 2]; k--) {
                                if (k == self_position[i * 2]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[k][self_position[i * 2 + 1] + py[j]] == brick ||
                                    state[k][self_position[i * 2 + 1] + py[j]] == steel)
                                    break;
                            }
                            break;
                        }
                    }
                } else {
                    for (int m = self_position[i * 2 + 1] - 1; m >= 0; m--) {
                        if (forestmap[self_position[i * 2] + px[j]][m] > threshold[self_position[i * 2] + px[j]][m][0]) {
                            for (int k = m; k <= self_position[i * 2 + 1]; k++) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[self_position[i * 2] + px[j]][k] == brick ||
                                    state[self_position[i * 2] + px[j]][k] == steel)
                                    break;
                            }
                            break;
                        }
                    }
                    for (int m = self_position[i * 2 + 1] + 1; m < 9; m++) {
                        if (forestmap[self_position[i * 2] + px[j]][m] > threshold[self_position[i * 2] + px[j]][m][0]) {
                            for (int k = m; k >= self_position[i * 2 + 1]; k--) {
                                if (k == self_position[i * 2 + 1]) {
                                    flag = 1;
                                    break;
                                }
                                if (state[self_position[i * 2] + px[j]][k] == brick ||
                                    state[self_position[i * 2] + px[j]][k] == steel)
                                    break;
                            }
                            break;
                        }
                    }
                }
                if (flag == 1) {
                    value[i][j] -= 200;
                }
            }
        }
    }
}

void EvaluateShot() {
    int temstate[10][10];
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j)temstate[i][j] = state[i][j];
    }
    int va[2][4] = {0};
    for (int TankId = 0; TankId < 2; ++TankId) {
        int tempx = enemy_position[2 * TankId],
        tempy = enemy_position[2 * TankId + 1];
        if (tempx == -1 || tempx == -2)continue;
        if (temstate[tempx][tempy] == 0 || temstate[tempx][tempy] == 2) temstate[tempx][tempy] += 7;
        
    }
    for (int TankId = 0; TankId < 2; ++TankId) {
        int tempx = enemy_position[2 * TankId],
        tempy = enemy_position[2 * TankId + 1];
        if (tempx == -1 || tempx == -2)continue;
        for (int i = 0; i < 4; ++i) {
            int tx = tempx + px[i], ty = tempy + py[i];
            if (tx < 0 || tx > 8 || ty < 0 || ty > 8)continue;
            if (temstate[tx][ty] == 0 || temstate[tx][ty] == 2)temstate[tx][ty] += 3;
        }
    }
    for (int TankId = 0; TankId < 2; ++TankId) {
        if (self_position[2 * TankId] == -1)continue;
        int tex = self_position[2 * TankId],
        tey = self_position[2 * TankId + 1];
        if (temstate[tex][tey] == 7) {
            if (tex == enemy_position[0] && tey == enemy_position[1]) {
                if (stayinterval[0] < 1) {
                    value[TankId][8] += 300;
                    continue;
                }
            } else {
                if (stayinterval[1] < 1) {
                    value[TankId][8] += 300;
                    continue;
                }
            }
        }
        for (int ShotDire = 0; ShotDire < 4; ++ShotDire) {
            int tempx = self_position[2 * TankId],
            tempy = self_position[2 * TankId + 1];
            while (1) {
                tempx += px[ShotDire];
                tempy += py[ShotDire];
                if (tempx < 0 || tempy < 0 || tempx > 8 || tempy > 8)break;
                if (tempx == 4 && tempy == 8) {
                    if (myside) va[TankId][ShotDire] = 0;
                    else va[TankId][ShotDire] += 10000;
                    break;
                }
                if (tempx == 4 && tempy == 0) {
                    if (myside) va[TankId][ShotDire] += 10000;
                    else va[TankId][ShotDire] = 0;
                    break;
                }
                
                if (TankId == 0) {
                    if (tempx == self_position[2] && tempy == self_position[3]) {
                        va[TankId][ShotDire] -= 500;
                        break;
                    }
                } else {
                    if (tempx == self_position[0] && tempy == self_position[1]) {
                        va[TankId][ShotDire] -= 500;
                        break;
                    }
                }
                if (temstate[tempx][tempy] == 1) {
                    va[TankId][ShotDire] += EvaluateGrid(tempx, tempy);
                    guess_enemy();
                    int tmpx2 = tempx + px[ShotDire], tmpy2 = tempy + py[ShotDire];
                    while (tmpx2 >= 0 && tmpx2 <= 8 && tmpy2 >= 0 && tmpy2 <= 8) {
                        if (temstate[tmpx2][tmpy2] == 1 || temstate[tmpx2][tmpy2] == 4) break;
                        if (temstate[tmpx2][tmpy2] == 3 || temstate[tmpx2][tmpy2] == 5) {
                            if (ShotDire % 2 == 0) {
                                if (!valid(TankId, 1) && !valid(TankId, 3)) {
                                    va[TankId][ShotDire] -= 3000;
                                    break;
                                }
                            } else {
                                if (!valid(TankId, 0) && !valid(TankId, 2)) {
                                    va[TankId][ShotDire] -= 3000;
                                    break;
                                }
                            }
                        }
                        if (temstate[tmpx2][tmpy2] == 7 || temstate[tmpx2][tmpy2] == 9) {
                            va[TankId][ShotDire] -= 3000;
                            break;
                        }
                        if (temstate[tmpx2][tmpy2] == 2) {
                            if (TankId == 0 && enemy_position[2] == -2) {
                                if (forestmap[tmpx2][tmpy2] > threshold[tmpx2][tmpy2][1]) {
                                    va[TankId][ShotDire] -= 3000;
                                    break;
                                }
                            } else if (TankId == 1 && enemy_position[0] == -2) {
                                if (forestmap[tmpx2][tmpy2] > threshold[tmpx2][tmpy2][1]) {
                                    va[TankId][ShotDire] -= 3000;
                                    break;
                                }
                            }
                            
                        }
                        tmpx2 += px[ShotDire];
                        tmpy2 += py[ShotDire];
                    }
                    break;
                }
                if (temstate[tempx][tempy] == 4) {
                    va[TankId][ShotDire] = 0;
                    break;
                }
                if (temstate[tempx][tempy] == 7 || temstate[tempx][tempy] == 9) {
                    if (temstate[tempx][tempy] == 9) {
                        if (self_position[2 * TankId] == tempx) {
                            if (myside == 1 && self_position[2 * TankId + 1] - 1 == tempy) {
                                value[TankId][0] += 1400;
                                break;
                            }
                            if (myside == 0 && self_position[2 * TankId + 1] + 1 == tempy) {
                                value[TankId][2] += 1400;
                                break;
                            }
                        }
                        if (self_position[2 * TankId + 1] == tempy) {
                            if (self_position[2 * TankId] - 1 == tempx) {
                                value[TankId][3] += 1400;
                                break;
                            }
                            if (self_position[2 * TankId] + 1 == tempx) {
                                value[TankId][1] += 1400;
                                break;
                            }
                        }
                    }
                    if (shoot_cnt[TankId] == 0) {
                        if ((myside == 0 && self_position[2 * TankId + 1] == 8) ||
                            (myside == 1 && self_position[2 * TankId + 1] == 0)) {
                            value[TankId][1] += 300;
                            value[TankId][3] += 300;
                        } else va[TankId][ShotDire] += 300;
                    } else {
                        if (enemy_position[0] == tempx && enemy_position[1] == tempy) {
                            if (pre_enemy_action[0] >= 4);
                            else {
                                int tx, ty, dis1, dis2;
                                if (ShotDire % 2 == 0) {
                                    tx = tempx + px[1];
                                    ty = tempy + py[1];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][1] += 300;
                                    tx = tempx + px[3];
                                    ty = tempy + py[3];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][3] += 300;
                                    if (dis1 > dis2)value[TankId][3] += 100;
                                    else value[TankId][1] += 100;
                                } else {
                                    tx = tempx + px[0];
                                    ty = tempy + py[0];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][0] += 300;
                                    tx = tempx + px[2];
                                    ty = tempy + py[2];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][2] += 300;
                                    if (dis1 > dis2)value[TankId][2] += 100;
                                    else value[TankId][0] += 100;
                                }
                            }
                        }
                        if (enemy_position[2] == tempx && enemy_position[3] == tempy) {
                            if (pre_enemy_action[1] >= 4);
                            else {
                                int tx, ty, dis1, dis2;
                                if (ShotDire % 2 == 0) {
                                    tx = tempx + px[1];
                                    ty = tempy + py[1];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][1] += 300;
                                    tx = tempx + px[3];
                                    ty = tempy + py[3];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][3] += 300;
                                    if (dis1 > dis2)value[TankId][3] += 100;
                                    else value[TankId][1] += 100;
                                } else {
                                    tx = tempx + px[0];
                                    ty = tempy + py[0];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][0] += 300;
                                    tx = tempx + px[2];
                                    ty = tempy + py[2];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][2] += 300;
                                    if (dis1 > dis2)value[TankId][2] += 100;
                                    else value[TankId][0] += 100;
                                }
                            }
                        }
                    }
                    break;
                }
                if (temstate[tempx][tempy] == 2) {
                    guess_enemy();
                    if (forestmap[tempx][tempy] > threshold[tempx][tempy][2]) {
                        if (shoot_cnt[TankId] == 0)va[TankId][ShotDire] += 250;
                        else {
                            int mx = self_position[2 * TankId], my = self_position[2 * TankId + 1];
                            int flag = 0;
                            while (1) {
                                mx += px[ShotDire];
                                my += py[ShotDire];
                                if (mx == tempx && my == tempy)break;
                                if (mx < 0 || mx > 8 || my < 0 || my > 8)break;
                                if (pre_state[mx][my] != temstate[mx][my]) {
                                    if (ShotDire + 4 == pre_self_action[TankId]) {
                                        if (flag == 0)flag = 1;
                                        else if (flag == 1)flag = 2;
                                    } else flag = 2;
                                }
                            }
                            if (flag != 2) {
                                int tx, ty, dis1, dis2;
                                if (ShotDire % 2 == 0) {
                                    tx = tempx + px[1];
                                    ty = tempy + py[1];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][1] += 300;
                                    tx = tempx + px[3];
                                    ty = tempy + py[3];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][3] += 300;
                                    if (dis1 > dis2)value[TankId][3] += 100;
                                    else value[TankId][1] += 100;
                                } else {
                                    tx = tempx + px[0];
                                    ty = tempy + py[0];
                                    if (myside == 0) dis1 = abs(tx - 4) + 8 - ty;
                                    else dis1 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][0] += 300;
                                    tx = tempx + px[2];
                                    ty = tempy + py[2];
                                    if (myside == 0) dis2 = abs(tx - 4) + 8 - ty;
                                    else dis2 = abs(tx - 4) + ty;
                                    if (tx >= 0 && tx <= 8 && ty >= 0 && ty <= 8)value[TankId][2] += 300;
                                    if (dis1 > dis2)value[TankId][2] += 100;
                                    else value[TankId][0] += 100;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    for (int i = 4; i < 8; ++i) {
        value[0][i] += va[0][i - 4];
        value[1][i] += va[1][i - 4];
    }
}

void makepolicy() {
    int temp[2] = {8, 8};
    for (int i = 0; i < 2; ++i) {
        int val = 1;
        for (int j = 0; j < 9; ++j) {
            if (valid(i, j) && value[i][j] >= val) {
                temp[i] = j;
                val = value[i][j];
            }
        }
    }
    action[0] = temp[0];
    action[1] = temp[1];
    if (action[0] == 8)action[0] = -1;
    if (action[1] == 8)action[1] = -1;
}

Json::Value all, input;

void ReadInput(istream &in, string &outData, string &outGlobalData) {
    Json::Reader reader;
    string inputString;
    do {
        getline(in, inputString);
    } while (inputString.empty());
#ifndef _BOTZONE_ONLINE
    // 猜测是单行还是多行
    char lastChar = inputString[inputString.size() - 1];
    if (lastChar != '}' && lastChar != ']') {
        // 第一行不以}或]结尾，猜测是多行
        string newString;
        do {
            getline(in, newString);
            inputString += newString;
        } while (newString != "}" && newString != "]");
    }
#endif
    reader.parse(inputString, all);
}

void init() {
    input = all["requests"];
    for (int i = 0; i < input.size(); i++) {
        if (i == 0)// read in the map information
        {
            myside = input[0u]["mySide"].asInt();
            if (!myside) {
                self_position[0] = 2;
                self_position[1] = 0;
                self_position[2] = 6;
                self_position[3] = 0;
                enemy_position[0] = 6;
                enemy_position[1] = 8;
                enemy_position[2] = 2;
                enemy_position[3] = 8;
            } else {
                self_position[0] = 6;
                self_position[1] = 8;
                self_position[2] = 2;
                self_position[3] = 8;
                enemy_position[0] = 2;
                enemy_position[1] = 0;
                enemy_position[2] = 6;
                enemy_position[3] = 0;
            }
            for (unsigned j = 0; j < 3; j++) {
                int x = input[0u]["brickfield"][j].asInt();
                for (int k = 0; k < 27; k++)state[k % 9][j * 3 + k / 9] |= (!!((1 << k) & x)) * brick;
            }
            for (unsigned j = 0; j < 3; j++) {
                int x = input[0u]["forestfield"][j].asInt();
                for (int k = 0; k < 27; k++)state[k % 9][j * 3 + k / 9] |= (!!((1 << k) & x)) * forest;
            }
            for (unsigned j = 0; j < 3; j++) {
                int x = input[0u]["steelfield"][j].asInt();
                for (int k = 0; k < 27; k++)state[k % 9][j * 3 + k / 9] |= (!!((1 << k) & x)) * steel;
            }
            for (unsigned j = 0; j < 3; j++) {
                int x = input[0u]["waterfield"][j].asInt();
                for (int k = 0; k < 27; k++)state[k % 9][j * 3 + k / 9] |= (!!((1 << k) & x)) * water;
            }
        } else// update enemy_position
        {
            for (int j = 0; j < 2; ++j)pre_enemy_action[j] = input[i]["action"][j].asInt();
            for (int j = 0; j < 2; j++) {
                pre_enemy_position[2 * j] = enemy_position[2 * j];
                pre_enemy_position[2 * j + 1] = enemy_position[2 * j + 1];
                if (input[i]["final_enemy_positions"][2 * j].asInt() == -2) {
                    if (enemyenterforest[2 * j] != -3) {
                        interval[j]++;
                    } else {
                        int tempx, tempy, f = 0;
                        for (int k = 0; k < 4; ++k) {
                            int tx = enemy_position[2 * j] + px[k], ty = enemy_position[2 * j + 1] + py[k];
                            if (tx < 0 || tx > 8 || ty < 0 || ty > 8)continue;
                            if (state[tx][ty] == 2) {
                                if (f == 0) {
                                    tempx = tx;
                                    tempy = ty;
                                    f = 1;
                                } else f = 2;
                            }
                        }
                        if (f == 1) {
                            enemy_position[2 * j] = tempx;
                            enemy_position[2 * j + 1] = tempy;
                        } else {
                            enemyenterforest[2 * j] = enemy_position[2 * j];
                            enemyenterforest[2 * j + 1] = enemy_position[2 * j + 1];
                            enemy_position[2 * j] = enemy_position[2 * j + 1] = -2;
                            interval[j]++;
                        }
                    }
                } else {
                    if (enemy_position[2 * j] == input[i]["final_enemy_positions"][2 * j].asInt() &&
                        enemy_position[2 * j + 1] == input[i]["final_enemy_positions"][2 * j + 1].asInt())
                        stayinterval[j]++;
                    else stayinterval[j] = 0;
                    enemy_position[2 * j] = input[i]["final_enemy_positions"][2 * j].asInt();
                    enemy_position[2 * j + 1] = input[i]["final_enemy_positions"][2 * j + 1].asInt();
                    interval[j] = 0;
                    enemyenterforest[2 * j] = enemyenterforest[2 * j + 1] = -3;
                }
            }
            for (int k = 0; k < 9; ++k) {
                for (int j = 0; j < 9; ++j)pre_state[k][j] = state[k][j];
            }
            for (int j = 0; j < input[i]["destroyed_blocks"].size(); j += 2) {
                int x = input[i]["destroyed_blocks"][j].asInt();
                int y = input[i]["destroyed_blocks"][j + 1].asInt();
                state[x][y] = 0;
            }
        }
    }
    input = all["responses"];
    for (int i = 0; i < input.size(); i++)//update self state
    {
        for (int j = 0; j < 2; j++) {
            if (i >= 1) {
                pre_self_action[j] = input[i - 1][j].asInt();
            }
            int x = input[i][j].asInt();
            if (x >= 4) {
                shoot_cnt[j]++;
            } else shoot_cnt[j] = 0;
            if (x == -1 || x >= 4)continue;
            self_position[2 * j] += px[x];
            self_position[2 * j + 1] += py[x];
        }
    }
}

int main() {
    Json::Reader reader;
    Json::Value output(Json::objectValue);
    Json::Value response(Json::arrayValue);
    string data, globaldata;
    
    ReadInput(cin, data, globaldata);
    
    init();
    
    shortest_route(self_position[0], self_position[1], 0);
    shortest_route(self_position[2], self_position[3], 1);
    /*
     for(int i=0;i<9;i++)
     cout<<value[1][i]<<" ";
     cout<<endl;
     */
    EvaluateShot();
    
    diagonal();
    guess_diagonal();
    
    makepolicy();
    
    response[0U] = action[0];
    response[1U] = action[1];
    
    output["response"] = response;
    
    Json::FastWriter writer;
    cout << writer.write(output);
    
    return 0;
}
