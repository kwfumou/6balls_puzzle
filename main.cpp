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

const int num_color = 3;
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

int pileBall[num_width][num_height]; // �ς܂�Ă���ʂ̐F
int lastBall[num_width][num_height] = { -1 }; // �Ō�̕\�������{�[��
int score = 0;

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

struct mBall {
    int num_side;
    int num_ver;
    int dir;
};

// unionfind
struct UnionFind {
    vector<int> par; // par[i]:i�̐e�̔ԍ��@(��) par[3] = 2 : 3�̐e��2
    vector<int> siz;

    UnionFind(int N) : par(N), siz(N) { //�ŏ��͑S�Ă����ł���Ƃ��ď�����
        for (int i = 0; i < N; i++) par[i] = i;
        for (int i = 0; i < N; i++) siz[i] = 1;
    }

    int root(int x) { // �f�[�^x��������؂̍����ċA�œ���Froot(x) = {x�̖؂̍�}
        if (par[x] == x) return x;
        return par[x] = root(par[x]);
    }

    void unite(int x, int y) { // x��y�̖؂𕹍�
        int rx = root(x); //x�̍���rx
        int ry = root(y); //y�̍���ry
        if (rx == ry) return; //x��y�̍�������(=�����؂ɂ���)���͂��̂܂�
        if (siz[ry] < siz[rx]) swap(rx, ry);
        siz[ry] += siz[rx];
        par[rx] = ry; //x��y�̍��������łȂ�(=�����؂ɂȂ�)���Fx�̍�rx��y�̍�ry�ɂ���
    }

    bool same(int x, int y) { // 2�̃f�[�^x, y��������؂������Ȃ�true��Ԃ�
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

void drawBalls(bool sleep = false) {
    resetScreen();
    for (int i = 0; i < num_width; i++) {
        for (int j = 0; j < num_height; j++) {

            const int pileColor = pileBall[i][j];

            if (pileColor == none) continue;
            DrawGraph(side_location[i], vertical_location[j], ghHandle[pileColor], TRUE);
        }
    }
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

    // ���ƉE�ɋ�������ꍇ�A�E�ړ��ɕϊ�
    if (ball.num_ver < num_height - 1 && ball.num_side < num_width - 1) {
        if (pileBall[ball.num_side + 1][ball.num_ver] && pileBall[ball.num_side][ball.num_ver + 1]) {
            ball.num_ver -= 1;
            //dir = right;
        }
    }

    // ���ƍ��ɋ�������ꍇ�A���ړ��ɕϊ�
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
        // struct�̏�������
        ball.color = rand() % num_color + 2;
    }
    else {
        ball.color = color + 2;
    }
   
    ball.num_side = num_width / 2;
    ball.num_ver = 0;

    // �ʒu�̎w��
    pileBall[ball.num_side][ball.num_ver] = ball.color;
}

bool fallJudge() {
    for (int S = 0; S < num_width; S++) {
        for (int V = 0; V < num_height - 1; V++) {
            if (pileBall[S][V] == none) continue;

            if (V < num_height - 1) {
                if (pileBall[S][V + 1] != none) return true;
            }
            if (S != num_width - 1 && V < num_height - 1) {
                if (pileBall[S + 1][V + 1] == none) return true;
            }
            if (S != 0 && V < num_height - 1) {
                if (pileBall[S - 1][V + 1] == none) return true;
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
    // change_location �g���Ă邯�ǁA����������V�������������Ă�
    
    for (int V = num_height - 2; V >= 0; V--) {
        int changed = 0;
        for (int S = 0; S < num_width; S++) {
            if (pileBall[S][V] == none) continue;

            ball.num_side = S;
            ball.num_ver = V;
            ball.color = pileBall[S][V];

            if (V < num_height - 1) {

                // �^���ɂ���
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
                         // ���ɉ����Ȃ�
                        changed += change_location(down);
                    }
                    else if (pileBall[S][V + 1] == none && pileBall[S - 1][V + 1] != none && pileBall[S + 1][V + 1] == none) {
                        // �����ɂ���
                        changed += change_location(low_right);
                    }
                    else if (pileBall[S][V + 1] == none && pileBall[S - 1][V + 1] == none && pileBall[S + 1][V + 1] != none) {
                        // �E���ɂ���
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

        // S,V�͈ړ����Ă��܂��Ă��邩��A�����̃R�[�h�͈Ӗ����Ȃ��B�ړ�����L�^�ł���΁A������\
        for (int S = 0; S < num_width; S++) {
            if (pileBall[S][V] == none) continue;

            ball.num_side = S;
            ball.num_ver = V;
            ball.color = pileBall[S][V];

            if (V < num_height - 1) {

                // �^���ɂ���
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
            }
        }
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

// 6�ȏ�̘A�������邩�ǂ����𔻒肵�A�A����������܂ŋʂ̏������J��Ԃ�
void eraseBall() {
    drawBalls();

    bool not_stay_f = false;
    bool not_stay_u = false;
    bool not_stay = false;

    // ��������
    not_stay_f = fallJudge();
    not_stay |= not_stay_f;

    // �A������
    UnionFind uf(num_width * num_height);
    re_union(&uf);
    not_stay_u = UnionJudge(&uf);
    not_stay |= not_stay_u;

    // ����OK

    // �ω����N����ꍇ�́A�ω��O�ɕ`��I�I�A���̂��Ǝ��Ԓ�~�I�I
    // ������̎��_�ł͕`��͕s�v�I�I
    // ���Ԓ�~�͗����A���ł��ꂼ��ŕK�v�����f���������ŁA���s
    while (not_stay) {
        // �`��
        //drawBalls();
        // ��ʂɕ`����Ă�����̂����ׂď���
        not_stay = false;

        // ���� change_location���g�p
        if (not_stay_f) {
            //DrawFormatString(100, 100, GetColor(0, 252, 0), "1111");

            fallBall();
            drawBalls(true);

        }
        // ����
        if ((!not_stay_f) && not_stay_u) {
            //DrawFormatString(100, 100, GetColor(0, 252, 0), "1111");

            UnionFind uf_(num_width * num_height);
            re_union(&uf_);
            unionErase(&uf_);
            drawBalls(true);

        }

        // �܂������邩���������瓪��
        //stay = fallJudge();
        //if (!stay) continue;
        // ��������
        not_stay_f = fallJudge();
        not_stay |= not_stay_f;

        // �܂������邩���������瓪��
        //stay = unionJudge();
        // �A������
        UnionFind uf_re(num_width * num_height);
        re_union(&uf_re);
        not_stay_u = UnionJudge(&uf_re);
        not_stay |= not_stay_u;

        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) {
            break;
        }
    }

    appearBalls();
}

void attackBall() {

    bool flag = false;

    // ��
    if (ball.num_side > 0) flag |= pileBall[ball.num_side - 1][ball.num_ver + 1];
    if (ball.num_side < num_width - 1) flag |= pileBall[ball.num_side + 1][ball.num_ver + 1];
    flag |= pileBall[ball.num_side][ball.num_ver + 1];

    // ���E
    if (ball.num_side > 1) flag |= pileBall[ball.num_side - 2][ball.num_ver];
    if (ball.num_side < num_width - 2) flag |= pileBall[ball.num_side + 2][ball.num_ver];
    
    if (flag) eraseBall();
}


void moveBall() {

    int dir_s = 0, dir_v = 0;
    int dir;

    // �L�[���͂ɂ��ψ�
    if (CheckHitKey(KEY_INPUT_LEFT) == 1) dir_s--;
    if (CheckHitKey(KEY_INPUT_RIGHT) == 1) dir_s++;
    if (CheckHitKey(KEY_INPUT_DOWN) == 1) dir_v++;

    // ���R�����ɂ��ψ�
    if (timeCount == 1) dir_v++;

    // ���x�x�N�g��
    if (dir_s == 0 && dir_v == 0) dir = freeze;
    if (dir_s == 0 && dir_v > 0) dir = down;
    if (dir_s < 0 && dir_v == 0) dir = left;
    if (dir_s > 0 && dir_v == 0) dir = right;
    if (dir_s < 0 && dir_v > 0) dir = low_left;
    if (dir_s > 0 && dir_v > 0) dir = low_right;

    // �ʒu�̈ړ��idir�����ۂɓ����������ɏ���������j
    change_location(dir);

    attackBall(); // �Ԃ������ꍇ��~���A�A���ɂ��������s��


}

void makeScreen() {
    SetDrawScreen(DX_SCREEN_BACK);
}

void makeGrayBalls() {
    for (int numGray = 0; numGray < num_width; numGray += 2) {
        pileBall[numGray][num_height - 1] = gray;
    }
}





void update() {
    // ���Ԃ��J�E���g
    timeCount++;
    timeCount %= div_speed;

    // ��ʂɕ`����Ă�����̂����ׂď���
    moveBall();

    // �`��
    drawBalls();

    

    // ����ʂ̓��e��\��ʂɔ��f������
    ScreenFlip();

    // �҂��Ȃ��Ə�������������̂ł����łQ�O�~���b�҂�
    WaitTimer(100);
}

void openPng() {
    for (int i = 0; i <= num_color; i++) {
        ghHandle[i+1] = LoadGraph(color_pathes[i].c_str());
    }
}

void startGame() {

    openPng();
    makeScreen();
    makeGrayBalls(); // �����ɊD�F�̋ʂ�~���l�߂�
    appearBalls(); // ���Â��3�̋ʂ��o���A���͈��

    while (true) {

        update();

        // Windows �V�X�e�����炭�������������
        if (ProcessMessage() == -1) break;

        // �d�r�b�L�[�������ꂽ�烋�[�v���甲����
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
    }
}

void test() {
    openPng();
    makeScreen();
    makeGrayBalls(); // �����ɊD�F�̋ʂ�~���l�߂�
    for (int i = 0; i < 5; i++) {
        pileBall[1 + i * 2][num_height - 2] = red;
    }
    for (int i = 0; i < 3; i++) {
        pileBall[i * 2][num_height - 3] = pink;
    }
    for (int i = 0; i < 2; i++) {
        pileBall[1 + i * 2][num_height - 4] = green;
    }
    
    drawBalls(true);
    appearBalls(0); // ���Â��3�̋ʂ��o���A���͈��
    while (true) {

        update();

        // Windows �V�X�e�����炭�������������
        if (ProcessMessage() == -1) break;

        // �d�r�b�L�[�������ꂽ�烋�[�v���甲����
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
    }
    
}

// WinMain�֐�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // �c�w���C�u��������������
    if (DxLib_Init() == -1) return -1;

    test();
    //startGame();

    // �c�w���C�u�����g�p�̏I������
    DxLib_End();

    // �\�t�g�̏I��
    return 0;
}