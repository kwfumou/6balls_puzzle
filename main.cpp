#include "DxLib.h"
#include <vector>
#include <math.h>
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

    UnionFind(int N) : par(N) { //�ŏ��͑S�Ă����ł���Ƃ��ď�����
        for (int i = 0; i < N; i++) par[i] = i;
    }

    int root(int x) { // �f�[�^x��������؂̍����ċA�œ���Froot(x) = {x�̖؂̍�}
        if (par[x] == x) return x;
        return par[x] = root(par[x]);
    }

    void unite(int x, int y) { // x��y�̖؂𕹍�
        int rx = root(x); //x�̍���rx
        int ry = root(y); //y�̍���ry
        if (rx == ry) return; //x��y�̍�������(=�����؂ɂ���)���͂��̂܂�
        par[rx] = ry; //x��y�̍��������łȂ�(=�����؂ɂȂ�)���Fx�̍�rx��y�̍�ry�ɂ���
    }

    bool same(int x, int y) { // 2�̃f�[�^x, y��������؂������Ȃ�true��Ԃ�
        int rx = root(x);
        int ry = root(y);
        return rx == ry;
    }
};

void drawBalls() {
    for (int i = 0; i < num_width; i++) {
        for (int j = 0; j < num_height; j++) {

            const int pileColor = pileBall[i][j];

            if (pileColor == none) continue;
            DrawGraph(side_location[i], vertical_location[j], ghHandle[pileColor], TRUE);
        }
    }
}

void change_location(int dir) {

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
}

void appearBalls() {

    // struct�̏�������
    ball.color = rand() % num_color + 2;
    ball.num_side = num_width / 2;
    ball.num_ver = 0;

    // �ʒu�̎w��
    pileBall[ball.num_side][ball.num_ver] = ball.color;
}

bool fallJudge() {

    // �`��
    //drawBalls();

    for (int i = 0; i < num_width; i++) {
        for (int j = 1; j < num_height; j++) {

        }
    }

    // ��ʂɕ`����Ă�����̂����ׂď���
    //ClearDrawScreen();
}

bool unionJudge() {

    // �`��
    //drawBalls();




    // ��ʂɕ`����Ă�����̂����ׂď���
    //ClearDrawScreen();
}

// 6�ȏ�̘A�������邩�ǂ����𔻒肵�A�A����������܂ŋʂ̏������J��Ԃ�
void eraseBall() {

    bool stay = true;

    // ��������
    stay = fallJudge();

    // �A������
    stay = unionJudge();

    // ����OK

    // �ω����N����ꍇ�́A�ω��O�ɕ`��I�I�A���̂��Ǝ��Ԓ�~�I�I
    // ������̎��_�ł͕`��͕s�v�I�I
    // ���Ԓ�~�͗����A���ł��ꂼ��ŕK�v�����f���������ŁA���s
    while (!stay) {

        stay = true;

        // ���� change_location���g�p


        // ����


        // �܂������邩���������瓪��
        //stay = fallJudge();
        //if (!stay) continue;

        // �܂������邩���������瓪��
        //stay = unionJudge();
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
    if (ball.num_side < 17) flag |= pileBall[ball.num_side + 2][ball.num_ver];
    
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
    ClearDrawScreen();

    moveBall();

    // �`��
    drawBalls();

    // ����ʂ̓��e��\��ʂɔ��f������
    ScreenFlip();

    // �҂��Ȃ��Ə�������������̂ł����łQ�O�~���b�҂�
    WaitTimer(100);
}

void openPng() {
    ghHandle[1] = LoadGraph("gray.png");
    ghHandle[2] = LoadGraph("red.png");
    ghHandle[3] = LoadGraph("pink.png");
    ghHandle[4] = LoadGraph("green.png");
    ghHandle[5] = LoadGraph("orange.png");
    ghHandle[6] = LoadGraph("cyan.png");
    ghHandle[7] = LoadGraph("brown.png");
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

}

// WinMain�֐�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // �c�w���C�u��������������
    if (DxLib_Init() == -1) return -1;

    //test();
    startGame();

    // �c�w���C�u�����g�p�̏I������
    DxLib_End();

    // �\�t�g�̏I��
    return 0;
}