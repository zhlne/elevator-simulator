#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>

// �]�w�̤j�ШD�ơB�Ӽh�ƻP���v�O������
#define MAX_REQUESTS 100
#define MAX_FLOOR 10
#define MAX_HISTORY 100

// ���Ȭ������c�G�q���@�h����@�h
typedef struct {
    int from;
    int to;
} TaskHistory;

// �q�赲�c�GID�B�ثe�Ӽh�B��V�B���v���ȡB��������
typedef struct {
    int id;
    int current_floor;
    int direction;
    TaskHistory history[MAX_HISTORY];
    int history_count;
} Elevator;

// �q��ШD���c�G�_�W�Ӽh�B�����q��B�O�_����
typedef struct {
    int id;
    int from_floor;
    int to_floor;
    int assigned_elevator;
    int completed;
} ElevatorRequest;

// �����ШD�}�C�P�q��}�C�]�ⳡ�q��^
ElevatorRequest requests[MAX_REQUESTS];
int request_count = 0;
Elevator elevators[2] = {
    {1, 1, 0, {}, 0},            // �q��1�G�q1�ӥX�o
    {2, MAX_FLOOR, 0, {}, 0}     // �q��2�G�q�̰��ӥX�o
};

// �{�ɰϥΩ�P�B�ʵe���
CRITICAL_SECTION cs;

// �s�W�@���ШD
void addRequest(int from, int to) {
    requests[request_count].id = request_count + 1;
    requests[request_count].from_floor = from;
    requests[request_count].to_floor = to;
    requests[request_count].assigned_elevator = 0;
    requests[request_count].completed = 0;
    request_count++;
}

// �p���Ӽƪ�����t��
int abs_diff(int a, int b) {
    return a > b ? a - b : b - a;
}

// �����ШD���̱��񪺹q��
void assignRequests() {
    for (int i = 0; i < request_count; i++) {
        int d1 = abs_diff(elevators[0].current_floor, requests[i].from_floor);
        int d2 = abs_diff(elevators[1].current_floor, requests[i].from_floor);
        // ������@�x�q��Z���̪�
        if (d1 <= d2) {
            requests[i].assigned_elevator = 1;
        } else {
            requests[i].assigned_elevator = 2;
        }
    }
}

// �H ASCII �e�X�ثe��x�q�誺��m
void drawElevators(Elevator e1, Elevator e2) {
    system("cls");  // �M�ŵe��
    printf("=== �q������� ASCII �ʵe ===\n");
    for (int floor = MAX_FLOOR; floor >= 1; floor--) {
        printf("�Ӽh %2d |", floor);
        if (e1.current_floor == floor) printf(" [E1]");
        else printf("     ");
        if (e2.current_floor == floor) printf(" [E2]");
        else printf("     ");
        printf("\n");
    }
    printf("------------------------------\n");
}

// ����q�貾�ʨ���w�Ӽh�]���ʵe�^
void moveElevator(Elevator *e, int target_floor) {
    while (e->current_floor != target_floor) {
        EnterCriticalSection(&cs);
        drawElevators(elevators[0], elevators[1]);
        LeaveCriticalSection(&cs);
        if (e->current_floor < target_floor) {
            e->current_floor++;
            e->direction = 1;
        } else {
            e->current_floor--;
            e->direction = -1;
        }
        Sleep(500);  // �������ʮɶ�
    }
    // ��F�ᰱ��
    e->direction = 0;
    EnterCriticalSection(&cs);
    drawElevators(elevators[0], elevators[1]);
    LeaveCriticalSection(&cs);
    Sleep(500);  // ���d�ɶ�
}

// ������@�q��]�Ω�h������^
unsigned __stdcall simulateElevator(void *param) {
    Elevator *e = (Elevator *)param;
    for (int i = 0; i < request_count; i++) {
        // �B�z�ݩ�o�x�q�誺����
        if (requests[i].assigned_elevator == e->id && !requests[i].completed) {
            // �e��������
            moveElevator(e, requests[i].from_floor);
            EnterCriticalSection(&cs);
            printf("�q�� %d �b�Ӽh %d ������ -> �e���Ӽh %d\n",
                   e->id, requests[i].from_floor, requests[i].to_floor);
            LeaveCriticalSection(&cs);
            Sleep(500);
            // �e���ت��a
            moveElevator(e, requests[i].to_floor);
            requests[i].completed = 1;

            // �[�J���v�O��
            e->history[e->history_count].from = requests[i].from_floor;
            e->history[e->history_count].to = requests[i].to_floor;
            e->history_count++;
        }
    }
    _endthreadex(0);  // ���������
    return 0;
}

// ��ܥ��Ⱦ��v�O��
void printHistory(Elevator *e) {
    printf("�q�� %d ���Ȭ����G\n", e->id);
    for (int i = 0; i < e->history_count; i++) {
        printf("  ���� %d�G�q %d �� �� %d ��\n", i + 1, e->history[i].from, e->history[i].to);
    }
}

// �D�{��
int main() {
    InitializeCriticalSection(&cs);  // ��l���{�ɰ�

    int from, to, more = 1;

    printf("== ���q������� + ���Ȭ����]ASCII �ʵe�^==\n");

    // ���ƿ�J���ȽШD
    while (more) {
        printf("��J�_�I�P���I�Ӽh�]�Ҧp 1 3�^�G");
        scanf("%d %d", &from, &to);
        addRequest(from, to);
        printf("�~���J�H1=�O, 0=�_�G");
        scanf("%d", &more);
    }

    assignRequests();  // ��������

    // �}�Ҩ�Ӱ����������x�q��
    HANDLE threads[2];
    threads[0] = (HANDLE)_beginthreadex(NULL, 0, simulateElevator, &elevators[0], 0, NULL);
    threads[1] = (HANDLE)_beginthreadex(NULL, 0, simulateElevator, &elevators[1], 0, NULL);

    // ���ݨ�Ӱ��������
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);

    printf("\n���������A�@ %d ���ШD�C\n\n", request_count);

    // ��ܨ�x�q�誺���Ⱦ��v
    printHistory(&elevators[0]);
    printHistory(&elevators[1]);

    DeleteCriticalSection(&cs);  // �����{�ɰ�
    system("pause");  // ���ݨϥΪ���������
    return 0;
}
