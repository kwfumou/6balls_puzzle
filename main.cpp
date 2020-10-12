#include "DxLib.h"
#include <vector>
#include <math.h>
#include <string>
using namespace std;

const int freeze = 0;
const int down = 1;
const int left = 2;
const int right = 3;
const int low_left = 4;
const int low_right = 5;

int timeCount = 0;
const int div_speed = 7;

const int num_width = 19;
const int num_height = 18;
const int side_location[] = { 150, 165, 181, 196, 212, 227, 243, 258, 274, 289, 305, 320, 336, 351, 367, 382, 398, 413, 429 };
const int vertical_location[] = { 8, 34, 60, 86, 112, 138, 164, 190, 216, 242, 268, 294, 320, 346, 372, 398, 424, 450 };

const int num_color = 6;
const string color_pathes[] = {"gray.png", "red.png", "pink.png", "green.png", "orange.png", "cyan.png", "brown.png"};

const int none = 0;
const int gray = 1;
const int red = 2;
const int pink = 3;
const int green = 4;
const int orange = 5;
const int cyan = 6;
const int brown = 7;

int ghHandle[num_color + 2];
int ghHaikei;
int ghWaku;
int ghGameover;

int pileBall[num_width][num_height]; // 積まれている玉の色
int lastBall[num_width][num_height] = { -1 }; // 最後の表示したボール
int score = 0;

bool gameOver = false;

void print(int message) {
    DrawFormatString(0, 0, GetColor(255, 0, 255), "%d", message);
    ScreenFlip();
}

struct Ball {
    int num_side;
    int num_ver;
    int color;
};
Ball ball;

struct tryBalls {
    Ball oneBall[3];
    int state; // 0が上向き、1が下向き
    int num_side;
    int num_ver;

    void put_ball() {

        oneBall[0].num_side = num_side;
        if (state == 0) {
            oneBall[1].num_side = num_side - 1;
            oneBall[2].num_side = num_side + 1;

            oneBall[0].num_ver = num_ver - 1;
            oneBall[1].num_ver = num_ver;
            oneBall[2].num_ver = num_ver;
        }
        else {
            oneBall[1].num_side = num_side + 1;
            oneBall[2].num_side = num_side - 1;

            oneBall[0].num_ver = num_ver;
            oneBall[1].num_ver = num_ver - 1;
            oneBall[2].num_ver = num_ver - 1;
        }
    }

    void init_(int S, int V) {
        state = 0;

        num_side = S;
        num_ver = V;

        put_ball();
    }

    void roll(int dir) {
        (state += 1) %= 2;

        Ball zero = oneBall[0];
        if (dir == right) {
            oneBall[0] = oneBall[2];
            oneBall[2] = oneBall[1];
            oneBall[1] = zero;
        }
        else if (dir == left) {
            oneBall[0] = oneBall[1];
            oneBall[1] = oneBall[2];
            oneBall[2] = zero;
        }

        if (state == 0) {
            num_side = oneBall[0].num_side;
            num_ver = oneBall[0].num_ver + 1;
        }
        else {
            num_side = oneBall[0].num_side;
            num_ver = oneBall[0].num_ver;
        }
    }
};
tryBalls tball;

// unionfind
struct UnionFind {
    vector<int> par; // par[i]:iの親の番号　(例) par[3] = 2 : 3の親が2
    vector<int> siz;

    UnionFind(int N) : par(N), siz(N) { //最初は全てが根であるとして初期化
        for (int i = 0; i < N; i++) par[i] = i;
        for (int i = 0; i < N; i++) siz[i] = 1;
    }

    int root(int x) { // データxが属する木の根を再帰で得る：root(x) = {xの木の根}
        if (par[x] == x) return x;
        return par[x] = root(par[x]);
    }

    void unite(int x, int y) { // xとyの木を併合
        int rx = root(x); //xの根をrx
        int ry = root(y); //yの根をry
        if (rx == ry) return; //xとyの根が同じ(=同じ木にある)時はそのまま
        if (siz[ry] < siz[rx]) swap(rx, ry);
        siz[ry] += siz[rx];
        par[rx] = ry; //xとyの根が同じでない(=同じ木にない)時：xの根rxをyの根ryにつける
    }

    bool same(int x, int y) { // 2つのデータx, yが属する木が同じならtrueを返す
        int rx = root(x);
        int ry = root(y);
        return rx == ry;
    }

    int size(int x) {
        return siz[root(x)];
    }
};

void resetScreen() {
    ClearDrawScreen();
    DrawFormatString(0, 0, GetColor(0, 255, 0), "%d", score);
}

void draw_3Balls() {

    for (int i = 0; i < 3; i++) {
        DrawGraph(side_location[tball.oneBall[i].num_side], vertical_location[tball.oneBall[i].num_ver], ghHandle[tball.oneBall[i].color], TRUE);
    }
}

void drawBalls(bool sleep = false) {
    resetScreen();

    DrawGraph(0, 41, ghHaikei, FALSE);
    DrawGraph(0, 41, ghWaku, TRUE);

    for (int i = 0; i < num_width; i++) {
        for (int j = 0; j < num_height; j++) {

            const int pileColor = pileBall[i][j];

            if (pileColor == none) continue;
            
            // ボールが縦に連結したときは、接する場所が重ならないように調整する
            if (j < num_height - 1 && pileBall[i][j + 1] != none) {
                DrawGraph(side_location[i], vertical_location[j] - 4, ghHandle[pileColor], TRUE);
            }
            else {
                DrawGraph(side_location[i], vertical_location[j], ghHandle[pileColor], TRUE);
            }
        }
    }

    if (!sleep) draw_3Balls();

    ScreenFlip();
    if (sleep && lastBall != pileBall) {
        WaitTimer(200);
    }
    for (int i = 0; i < num_width; i++) {
        memcpy(lastBall[i], pileBall[i], sizeof(pileBall[i]));
    }
}

bool change_location(int dir) {
    int memo_h = ball.num_side;
    int memo_w = ball.num_ver;
    pileBall[ball.num_side][ball.num_ver] = none;
    

    if ((dir == down && ball.num_ver < num_height - 1) || (dir == low_left && ball.num_side == 0) || (dir == low_right && ball.num_side == num_width - 1)) {
        ball.num_ver += 1;
        //dir = down;
    }
    else if (dir == left && ball.num_side > 0) {
        ball.num_side -= 1;
        //dir = left;
    }
    else if (dir == right && ball.num_side < num_width - 1) {
        ball.num_side += 1;
        //dir = right;
    }
    else if (dir == low_left && ball.num_ver < num_height - 1 && ball.num_side > 0) {
        ball.num_side -= 1;
        ball.num_ver += 1;

        //dir = low_left;
    }
    else if (dir == low_right && ball.num_ver < num_height - 1 && ball.num_side < num_width - 1) {
        ball.num_side += 1;
        ball.num_ver += 1;

        //dir = low_right;
    }

    // 下と右に球がある場合、右移動に変換
    if (ball.num_ver < num_height - 1 && ball.num_side < num_width - 1) {
        if (pileBall[ball.num_side + 1][ball.num_ver] && pileBall[ball.num_side][ball.num_ver + 1]) {
            ball.num_ver -= 1;
            //dir = right;
        }
    }

    // 下と左に球がある場合、左移動に変換
    if (ball.num_ver < num_height - 1 && ball.num_side > 0) {
        if (pileBall[ball.num_side - 1][ball.num_ver] && pileBall[ball.num_side][ball.num_ver + 1]) {
            ball.num_ver -= 1;
            //dir = left;
        }
    }

    pileBall[ball.num_side][ball.num_ver] = ball.color;
    return ((memo_h != ball.num_side) || (memo_w != ball.num_ver));
}

void appearBalls(int color = -1) {
    if (color == -1) {
        // structの書き換え
        ball.color = rand() % num_color + 2;
    }
    else {
        ball.color = color + 2;
    }
   
    ball.num_side = num_width / 2;
    ball.num_ver = 0;

    // 位置の指定
    pileBall[ball.num_side][ball.num_ver] = ball.color;
}

bool change_3Balls(int dir) {

    if (tball.num_side == 1) {
        if (dir == left) dir = freeze;
        if (dir == low_left) dir = down;
    }
    if (tball.num_side == num_width - 2) {
        if (dir == right) dir = freeze;
        if (dir == low_right) dir = down;
    }

    if (dir == down || dir == low_left || dir == low_right) tball.num_ver++;
    if (dir == left || dir == low_left) tball.num_side--;
    if (dir == right || dir == low_right) tball.num_side++;

    // 下と二個右に球がある場合、右移動に変換
    if (tball.num_ver < num_height - 1 && tball.num_side < num_width - 2) {
        if (pileBall[tball.num_side + 2][tball.num_ver] && pileBall[tball.num_side + 1][tball.num_ver + 1]) {
            tball.num_ver -= 1;
            //dir = right;
        }
    }

    // 下と二個左に球がある場合、左移動に変換
    if (tball.num_ver < num_height - 1 && tball.num_side > 1) {
        if (pileBall[tball.num_side - 2][tball.num_ver] && pileBall[tball.num_side - 1][tball.num_ver + 1]) {
            tball.num_ver -= 1;
            //dir = left;
        }
    }

    tball.put_ball();

    // 当たり判定
    if (tball.state == 0) {
        for (int S = -2; S <= 2; S++) {
            for (int V = -1; V <= 1; V++) {
                int sid = S + tball.num_side, ver = V + tball.num_ver;

                if (sid < 0 || sid >= num_width) continue;
                if (ver < 0 || ver >= num_height) continue;

                if (pileBall[sid][ver] != none) return true;
            }
        }
    }
    else {
        for (int S = -2; S <= 2; S++) {
            for (int V = -1; V <= 1; V++) {
                if (S == -2 && V == 1) continue;
                if (S == 2 && V == 1) continue;

                int sid = S + tball.num_side, ver = V + tball.num_ver;

                if (sid < 0 || sid >= num_width) continue;
                if (ver < 0 || ver >= num_height) continue;

                if (pileBall[sid][ver] != none) return true;
            }
        }
    }

    return false;
}

void appear_3Balls(int color_a = -1, int color_b = -1, int color_c = -1) {
    if (color_a == -1) {
        // structの書き換え
        for (int i = 0; i < 3; i++) tball.oneBall[i].color = rand() % num_color + 2;
    }
    else {
        tball.oneBall[0].color = color_a + 2;
        tball.oneBall[1].color = color_b + 2;
        tball.oneBall[2].color = color_c + 2;
    }

    tball.init_(num_width / 2, 1);

    // 位置の指定
    //for (int i = 0; i < 3; i++) pileBall[tball.ball[i].num_side][tball.ball[i].num_ver] = tball.ball[i].color;
}

bool fallJudge() {
    for (int S = 0; S < num_width; S++) {
        for (int V = 0; V < num_height - 1; V++) {
            if (pileBall[S][V] == none) continue;

            if (V < num_height - 1) {
                if (pileBall[S][V + 1] != none) return true;
            }
            if (S != num_width - 1 && V < num_height - 1) {

                if (S < num_width - 2) {
                    if (pileBall[S + 1][V + 1] == none && pileBall[S + 2][V] == none) return true;
                }
                else {
                    if (pileBall[S + 1][V + 1] == none) return true;
                }
            }
            if (S != 0 && V < num_height - 1) {

                if (S > 1) {
                    if (pileBall[S - 1][V + 1] == none && pileBall[S - 2][V] == none) return true;
                }
                else {
                    if (pileBall[S - 1][V + 1] == none) return true;
                }
            }
        }
    }

    return false;
}

int num_ball(int S, int V) {
    return S * num_height + V;
}

void re_union(UnionFind *uf) {
    
    for (int S = 0; S < num_width; S++) {
        for (int V = 0; V < num_height - 1; V++) {

            if (pileBall[S][V] == none) continue;

            if (S > 1) {
                if (pileBall[S][V] == pileBall[S - 2][V]) {
                    uf->unite(num_ball(S, V), num_ball(S - 2, V));
                }
            }
            if (S > 0) {
                if (V > 0) {
                    if (pileBall[S][V] == pileBall[S - 1][V - 1]) {
                        uf->unite(num_ball(S, V), num_ball(S - 1, V - 1));
                    }
                }
                if (V < num_height - 1) {
                    if (pileBall[S][V] == pileBall[S - 1][V + 1]) {
                        uf->unite(num_ball(S, V), num_ball(S - 1, V + 1));
                    }
                }
            }
        }
    }
}



void fallBall() {
    // change_location 使ってるけど、無理だから新しく書き直してね
    
    for (int V = num_height - 2; V >= 0; V--) {
        int changed = 0;
        for (int S = 0; S < num_width; S++) {
            if (pileBall[S][V] == none) continue;

            ball.num_side = S;
            ball.num_ver = V;
            ball.color = pileBall[S][V];

            if (V < num_height - 1) {

                // 真下にある
                if (pileBall[S][V + 1] != none) {
                    //int ran = rand() % 2;
                    if (S == num_width - 1) {
                        changed += change_location(left);
                    }
                    else if (S == 0) {
                        changed += change_location(right);
                    }
                    else if (pileBall[S - 1][V] != none) {
                        changed += change_location(right);
                    }
                    else {
                        changed += change_location(left);
                    }
                    continue;
                }
                
                if (S > 0 && S < num_width - 1) {
                    if (pileBall[S][V + 1] == none && pileBall[S - 1][V + 1] == none && pileBall[S + 1][V + 1] == none) {
                         // 下に何もない
                        changed += change_location(down);
                    }
                    else if (pileBall[S][V + 1] == none && pileBall[S - 1][V + 1] != none && pileBall[S + 1][V + 1] == none) {
                        // 左下にある
                        changed += change_location(low_right);
                    }
                    else if (pileBall[S][V + 1] == none && pileBall[S - 1][V + 1] == none && pileBall[S + 1][V + 1] != none) {
                        // 右下にある
                        changed += change_location(low_left);
                    }
                }
                else if (S == 0) {
                    if (pileBall[S + 1][V + 1] == none) {
                        changed += change_location(down);
                    }
                }
                else if (S == num_width - 1) {
                    if (pileBall[S - 1][V + 1] == none) {
                        changed += change_location(down);
                    }
                }
            }

        }
        if(changed) drawBalls(true);
        
        changed = 0;

        if (V == num_width - 2) continue;

        V++;

        // S,Vは移動してしまっているから、したのコードは意味がない。移動先を記録できれば、横ずれ可能
        for (int S = 0; S < num_width; S++) {
            if (pileBall[S][V] == none) continue;

            ball.num_side = S;
            ball.num_ver = V;
            ball.color = pileBall[S][V];

            if (V < num_height - 1) {

                // 真下にある
                if (pileBall[S][V + 1] != none) {
                    //int ran = rand() % 2;
                    if (S == num_width - 1) {
                        changed += change_location(left);
                    }
                    else if (S == 0) {
                        changed += change_location(right);
                    }
                    else if (pileBall[S - 1][V] != none) {
                        changed += change_location(right);
                    }
                    else {
                        changed += change_location(left);
                    }
                }
            }
        }

        V--;

        if (changed) drawBalls(true);   
    }
}

void unionErase(UnionFind *uf) {
    for (int S = 0; S < num_width; S++) {
        for (int V = 0; V < num_height - 1; V++) {
            if (uf->size(num_ball(S, V)) >= 6) {
                pileBall[S][V] = none;
                score++;
            }
        }
    }
}

bool UnionJudge(UnionFind *uf) {
    //int sc = 0;

    for (int i = 0; i < num_width; i++) {
        for (int j = 0; j < num_height - 1; j++) {
            //score = max(score, uf->size(num_ball(i, j)));

            if (uf->size(num_ball(i, j)) >= 6) return true;
        }
    }
    return false;
}

// 6個以上の連結があるかどうかを判定し、連結が消えるまで玉の消去を繰り返す
void eraseBall() {
    drawBalls();

    bool not_stay_f = false;
    bool not_stay_u = false;
    bool not_stay = false;

    // 落下判定
    not_stay_f = fallJudge();
    not_stay |= not_stay_f;

    // 連結判定
    UnionFind uf(num_width * num_height);
    re_union(&uf);
    not_stay_u = UnionJudge(&uf);
    not_stay |= not_stay_u;

    // 上二つはOK

    // 変化が起こる場合は、変化前に描画！！、そのあと時間停止！！
    // ↑判定の時点では描画は不要！！
    // 時間停止は落下、消滅それぞれで必要か判断したうえで、執行
    while (not_stay) {
        // 描画
        //drawBalls();
        // 画面に描かれているものをすべて消す
        not_stay = false;

        // 落下 change_locationを使用
        if (not_stay_f) {
            //DrawFormatString(100, 100, GetColor(0, 252, 0), "1111");

            fallBall();
            drawBalls(true);

        }
        // 消滅
        if ((!not_stay_f) && not_stay_u) {
            //DrawFormatString(100, 100, GetColor(0, 252, 0), "1111");

            UnionFind uf_(num_width * num_height);
            re_union(&uf_);
            unionErase(&uf_);
            drawBalls(true);

        }

        // まだ落ちるか→落ちたら頭へ
        //stay = fallJudge();
        //if (!stay) continue;
        // 落下判定
        not_stay_f = fallJudge();
        not_stay |= not_stay_f;

        // まだ消えるか→消えたら頭へ
        //stay = unionJudge();
        // 連結判定
        UnionFind uf_re(num_width * num_height);
        re_union(&uf_re);
        not_stay_u = UnionJudge(&uf_re);
        not_stay |= not_stay_u;

        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) {
            break;
        }
    }

    //appearBalls();
    appear_3Balls();
}

void attackBall() {

    bool flag = false;

    // 下
    if (ball.num_side > 0) flag |= pileBall[ball.num_side - 1][ball.num_ver + 1];
    if (ball.num_side < num_width - 1) flag |= pileBall[ball.num_side + 1][ball.num_ver + 1];
    flag |= pileBall[ball.num_side][ball.num_ver + 1];

    // 左右
    if (ball.num_side > 1) flag |= pileBall[ball.num_side - 2][ball.num_ver];
    if (ball.num_side < num_width - 2) flag |= pileBall[ball.num_side + 2][ball.num_ver];
    
    if (flag) eraseBall();
}


void moveBall() {

    int dir_s = 0, dir_v = 0;
    int dir;

    // キー入力による変位
    if (CheckHitKey(KEY_INPUT_LEFT) == 1) dir_s--;
    if (CheckHitKey(KEY_INPUT_RIGHT) == 1) dir_s++;
    if (CheckHitKey(KEY_INPUT_DOWN) == 1) dir_v++;

    // 自然落下による変位
    if (timeCount == 1) dir_v++;

    // 速度ベクトル
    if (dir_s == 0 && dir_v == 0) dir = freeze;
    if (dir_s == 0 && dir_v > 0) dir = down;
    if (dir_s < 0 && dir_v == 0) dir = left;
    if (dir_s > 0 && dir_v == 0) dir = right;
    if (dir_s < 0 && dir_v > 0) dir = low_left;
    if (dir_s > 0 && dir_v > 0) dir = low_right;

    // 位置の移動（dirを実際に動いた向きに書き換える）
    change_location(dir);

    attackBall(); // ぶつかった場合停止し、連結による消去を行う


}

void move_3Balls() {
    int dir_s = 0, dir_v = 0;
    int dir;

    // キー入力による変位
    if (CheckHitKey(KEY_INPUT_LEFT) == 1) dir_s--;
    if (CheckHitKey(KEY_INPUT_RIGHT) == 1) dir_s++;
    if (CheckHitKey(KEY_INPUT_DOWN) == 1) dir_v++;

    // 自然落下による変位
    if (timeCount == 1) dir_v++;

    // 速度ベクトル
    if (dir_s == 0 && dir_v == 0) dir = freeze;
    if (dir_s == 0 && dir_v > 0) dir = down;
    if (dir_s < 0 && dir_v == 0) dir = left;
    if (dir_s > 0 && dir_v == 0) dir = right;
    if (dir_s < 0 && dir_v > 0) dir = low_left;
    if (dir_s > 0 && dir_v > 0) dir = low_right;

    // 回転
    if (CheckHitKey(KEY_INPUT_F)) tball.roll(right);
    if (CheckHitKey(KEY_INPUT_D)) tball.roll(left);

    // 位置の移動（dirを実際に動いた向きに書き換える)
    bool flag = change_3Balls(dir); // 当たり判定を返す

    if (flag) {

        // 3個のボールをpileBallに書き出す。
        for (int i = 0; i < 3; i++) pileBall[tball.oneBall[i].num_side][tball.oneBall[i].num_ver] = tball.oneBall[i].color;

        eraseBall();
    }
}

void makeScreen() {
    SetDrawScreen(DX_SCREEN_BACK);
}

void makeGrayBalls() {
    for (int numGray = 0; numGray < num_width; numGray += 2) {
        pileBall[numGray][num_height - 1] = gray;
    }
}

void endGame() {
    resetScreen();

    DrawGraph(0, 80, ghGameover, FALSE);
}

bool endJudge() {
    for (int S = 0; S < num_width; S++) {
        for (int V = 0; V < 6; V++) {
            if (pileBall[S][V] != none) return true;
        }
    }
    return false;
}

void update() {
    

    if (!gameOver) {
        // 時間をカウント
        timeCount++;
        timeCount %= div_speed;

        // 画面に描かれているものをすべて消す
        //moveBall();
        move_3Balls();

        // 描画
        drawBalls();

        // 終了判定
        gameOver = endJudge();
    }
    else {
        endGame();
    }

    // ゲームオーバーか判定


    // 裏画面の内容を表画面に反映させる
    ScreenFlip();

    // 待たないと処理が早すぎるのでここで２０ミリ秒待つ
    WaitTimer(100);
}

void openPng() {
    for (int i = 0; i <= num_color; i++) {
        ghHandle[i+1] = LoadGraph(color_pathes[i].c_str());
    }
    ghHaikei = LoadGraph("haikei.png");
    ghWaku = LoadGraph("waku.png");
    ghGameover = LoadGraph("gameover.png");
}

void startGame() {

    openPng();
    makeScreen();
    makeGrayBalls(); // そこに灰色の玉を敷き詰める
    appearBalls(); // いづれは3個の玉が出現、今は一個

    while (true) {

        update();

        // Windows システムからくる情報を処理する
        if (ProcessMessage() == -1) break;

        // ＥＳＣキーが押されたらループから抜ける
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
    }
}

void test() {
    openPng();
    makeScreen();
    makeGrayBalls(); // そこに灰色の玉を敷き詰める
    /*
    for (int i = 0; i < 5; i++) {
        pileBall[1 + i * 2][num_height - 2] = red;
    }
    for (int i = 0; i < 3; i++) {
        pileBall[i * 2][num_height - 3] = pink;
    }
    for (int i = 0; i < 2; i++) {
        pileBall[1 + i * 2][num_height - 4] = green;
    }
    */
    
    for (int i = 0; i < num_width; i += 2) {
        for (int j = 6; j < num_height - 1; j++) {
            pileBall[i][j] = (i + j) % num_color;
        }
    }

    drawBalls(true);
    //appearBalls(0); // いづれは3個の玉が出現、今は一個

    appear_3Balls();

    while (true) {

        update();

        // Windows システムからくる情報を処理する
        if (ProcessMessage() == -1) break;

        // ＥＳＣキーが押されたらループから抜ける
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
    }
    
}

// WinMain関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // ＤＸライブラリ初期化処理
    if (DxLib_Init() == -1) return -1;

    test();
    //startGame();

    // ＤＸライブラリ使用の終了処理
    DxLib_End();

    // ソフトの終了
    return 0;
}