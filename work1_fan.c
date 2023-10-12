#include <wiringPi.h>
#include <softPwm.h>
//모터의 핀 연걸
#define MOTOR_MT_P_PIN 4
#define MOTOR_MT_N_PIN 17
#define LEFT_ROTATE 1
#define RIGHT_ROTATE 2
//키패드의 핀 연결
#define KEYPAD_PB1 23
#define KEYPAD_PB2 24
#define KEYPAD_PB3 2
#define KEYPAD_PB4 3
#define KEYPAD_PB5 25
#define MAX_KEY_BT_NUM 5
//LED의 핀 연결
#define LED_PIN_1 5
#define LED_PIN_2 6
#define LED_PIN_3 13
#define LED_ON 1
#define LED_OFF 0
#define MAX_LED_NUM 3

const int KeypadTable[MAX_KEY_BT_NUM] = {KEYPAD_PB1, KEYPAD_PB2, KEYPAD_PB3, KEYPAD_PB4, KEYPAD_PB5};
const int LedPinTable[MAX_LED_NUM] = {LED_PIN_1, LED_PIN_2, LED_PIN_3};

// 키패드의 상태를 읽어오는 함수
int KeypadRead(void) {
    int nKeypadstate = 0;

    for(int i = 0; i < MAX_KEY_BT_NUM; i++) {
        if (!digitalRead(KeypadTable[i])) {
            nKeypadstate |= (1 << i);
        }
    }

    return nKeypadstate;
}

// LED를 제어하는 함수
void LedControl(int LedNum, int Cmd) {
    for (int i = 0; i < MAX_LED_NUM; i++) {
        digitalWrite(LedPinTable[i], (i == LedNum) ? Cmd : !Cmd);
    }
}

// 모터를 정지시키는 함수
void MotorStop() {
    softPwmWrite(MOTOR_MT_N_PIN, 0);
    softPwmWrite(MOTOR_MT_P_PIN, 0);
}

// 모터를 제어하는 함수
void MotorControl(unsigned char speed, unsigned char rotate) {
    if (rotate == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        softPwmWrite(MOTOR_MT_N_PIN, speed);
    } else if (rotate == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_N_PIN, LOW);
        softPwmWrite(MOTOR_MT_P_PIN, speed);
    }
}

// LED를 모두 끄는 함수
void off_led() {
    for(int j = 0; j < MAX_LED_NUM; j++) {
        LedControl(j, LED_OFF);
    }
}

int main() {
    if (wiringPiSetupGpio() == -1)
        return 1;

    int nKeypadstate;
    //이전의 회전 상태를 저장하는 rot와 풍속상태를 저장하는 past_i
    int rot = 0;
    int past_i = 0;

    // LED 핀을 출력으로 설정
    for (int i = 0; i < MAX_LED_NUM; i++) {
        pinMode(LedPinTable[i], OUTPUT);
    }

    // 키패드 핀을 입력으로 설정
    for (int i = 0; i < MAX_KEY_BT_NUM; i++) {
        pinMode(KeypadTable[i], INPUT);
    }

    // 모터 제어 핀 설정
    pinMode(MOTOR_MT_N_PIN, OUTPUT);
    pinMode(MOTOR_MT_P_PIN, OUTPUT);
    softPwmCreate(MOTOR_MT_N_PIN, 0, 100);
    softPwmCreate(MOTOR_MT_P_PIN, 0, 100);

    while (1) {
        //while문 내에서 키패드의 상태를 계속 입력받음
        nKeypadstate = KeypadRead();

        for (int i = 0; i < MAX_KEY_BT_NUM; i++) {
            if (nKeypadstate & (1 << i)) {
                if(i == 0) {
                    // 1번스위치는 선풍기 종료 버튼
                    off_led();
                    MotorStop();
                    past_i = i;
                } else if (i >= 1 && i <= 3) {
                    // 2~4번 스위치는 풍속을 제어하며, LED로 몇단인지 표시
                    //rot가 0일 경우는 RIGHT_ROTATE(시계방향)으로 회전, 1일 경우는 반대로
                    MotorControl(i * 20, (rot == 0) ? RIGHT_ROTATE : LEFT_ROTATE);
                    off_led();
                    LedControl(i - 1, LED_ON);
                    past_i = i;
                } else if (i == 4) {
                    // 5번 스위치는 회전 방향을 변경
                    rot = !rot; //회전 방향 변경을 위해 rot의 상태를 바꾼다
                    MotorStop();
                    delay(2000);//빠르게 회전하는 모터에게 멈출 시 까지 딜레이
                    MotorControl(past_i * 20, (rot == 0) ? RIGHT_ROTATE : LEFT_ROTATE);
                    off_led();
                    LedControl(past_i - 1, LED_ON);
                }
                break;
            }
        }
    }

    return 0;
}
