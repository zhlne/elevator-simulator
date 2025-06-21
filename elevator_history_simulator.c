#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>

// 設定最大請求數、樓層數與歷史記錄長度
#define MAX_REQUESTS 100
#define MAX_FLOOR 10
#define MAX_HISTORY 100

// 任務紀錄結構：從哪一層到哪一層
typedef struct {
    int from;
    int to;
} TaskHistory;

// 電梯結構：ID、目前樓層、方向、歷史任務、紀錄筆數
typedef struct {
    int id;
    int current_floor;
    int direction;
    TaskHistory history[MAX_HISTORY];
    int history_count;
} Elevator;

// 電梯請求結構：起訖樓層、指派電梯、是否完成
typedef struct {
    int id;
    int from_floor;
    int to_floor;
    int assigned_elevator;
    int completed;
} ElevatorRequest;

// 全部請求陣列與電梯陣列（兩部電梯）
ElevatorRequest requests[MAX_REQUESTS];
int request_count = 0;
Elevator elevators[2] = {
    {1, 1, 0, {}, 0},            // 電梯1：從1樓出發
    {2, MAX_FLOOR, 0, {}, 0}     // 電梯2：從最高樓出發
};

// 臨界區用於同步動畫顯示
CRITICAL_SECTION cs;

// 新增一筆請求
void addRequest(int from, int to) {
    requests[request_count].id = request_count + 1;
    requests[request_count].from_floor = from;
    requests[request_count].to_floor = to;
    requests[request_count].assigned_elevator = 0;
    requests[request_count].completed = 0;
    request_count++;
}

// 計算兩個數的絕對差值
int abs_diff(int a, int b) {
    return a > b ? a - b : b - a;
}

// 指派請求給最接近的電梯
void assignRequests() {
    for (int i = 0; i < request_count; i++) {
        int d1 = abs_diff(elevators[0].current_floor, requests[i].from_floor);
        int d2 = abs_diff(elevators[1].current_floor, requests[i].from_floor);
        // 比較哪一台電梯距離最近
        if (d1 <= d2) {
            requests[i].assigned_elevator = 1;
        } else {
            requests[i].assigned_elevator = 2;
        }
    }
}

// 以 ASCII 畫出目前兩台電梯的位置
void drawElevators(Elevator e1, Elevator e2) {
    system("cls");  // 清空畫面
    printf("=== 電梯模擬器 ASCII 動畫 ===\n");
    for (int floor = MAX_FLOOR; floor >= 1; floor--) {
        printf("樓層 %2d |", floor);
        if (e1.current_floor == floor) printf(" [E1]");
        else printf("     ");
        if (e2.current_floor == floor) printf(" [E2]");
        else printf("     ");
        printf("\n");
    }
    printf("------------------------------\n");
}

// 控制電梯移動到指定樓層（有動畫）
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
        Sleep(500);  // 模擬移動時間
    }
    // 到達後停止
    e->direction = 0;
    EnterCriticalSection(&cs);
    drawElevators(elevators[0], elevators[1]);
    LeaveCriticalSection(&cs);
    Sleep(500);  // 停留時間
}

// 模擬單一電梯（用於多執行緒）
unsigned __stdcall simulateElevator(void *param) {
    Elevator *e = (Elevator *)param;
    for (int i = 0; i < request_count; i++) {
        // 處理屬於這台電梯的任務
        if (requests[i].assigned_elevator == e->id && !requests[i].completed) {
            // 前往接乘客
            moveElevator(e, requests[i].from_floor);
            EnterCriticalSection(&cs);
            printf("電梯 %d 在樓層 %d 接乘客 -> 前往樓層 %d\n",
                   e->id, requests[i].from_floor, requests[i].to_floor);
            LeaveCriticalSection(&cs);
            Sleep(500);
            // 前往目的地
            moveElevator(e, requests[i].to_floor);
            requests[i].completed = 1;

            // 加入歷史記錄
            e->history[e->history_count].from = requests[i].from_floor;
            e->history[e->history_count].to = requests[i].to_floor;
            e->history_count++;
        }
    }
    _endthreadex(0);  // 結束執行緒
    return 0;
}

// 顯示任務歷史記錄
void printHistory(Elevator *e) {
    printf("電梯 %d 任務紀錄：\n", e->id);
    for (int i = 0; i < e->history_count; i++) {
        printf("  任務 %d：從 %d 樓 到 %d 樓\n", i + 1, e->history[i].from, e->history[i].to);
    }
}

// 主程式
int main() {
    InitializeCriticalSection(&cs);  // 初始化臨界區

    int from, to, more = 1;

    printf("== 雙電梯模擬器 + 任務紀錄（ASCII 動畫）==\n");

    // 重複輸入任務請求
    while (more) {
        printf("輸入起點與終點樓層（例如 1 3）：");
        scanf("%d %d", &from, &to);
        addRequest(from, to);
        printf("繼續輸入？1=是, 0=否：");
        scanf("%d", &more);
    }

    assignRequests();  // 指派任務

    // 開啟兩個執行緒模擬兩台電梯
    HANDLE threads[2];
    threads[0] = (HANDLE)_beginthreadex(NULL, 0, simulateElevator, &elevators[0], 0, NULL);
    threads[1] = (HANDLE)_beginthreadex(NULL, 0, simulateElevator, &elevators[1], 0, NULL);

    // 等待兩個執行緒結束
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);

    printf("\n模擬完成，共 %d 筆請求。\n\n", request_count);

    // 顯示兩台電梯的任務歷史
    printHistory(&elevators[0]);
    printHistory(&elevators[1]);

    DeleteCriticalSection(&cs);  // 釋放臨界區
    system("pause");  // 等待使用者關閉視窗
    return 0;
}
