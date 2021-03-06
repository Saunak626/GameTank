// GameTank.cpp: 定义应用程序的入口点。
//

#include "Main.h"

typedef enum difficult_s
{

    DIFFICULT_A = 6,
    DIFFICULT_B = 11,
    DIFFICULT_C = 16,
    DIFFICULT_D = 21,
    DIFFICULT_MIN = DIFFICULT_A,
    DIFFICULT_MAX = DIFFICULT_D
}difficult_t;

typedef struct game_ctrl_s
{
    BOOL            run;
    dir_t           dir;
    BOOL            fire;
    BOOL            auto_fire;
    int32           speed;
    difficult_t     difficult;
    int32           debug;
    int32           super;
}game_ctrl_t;

#define GAME_SPEED_MAX  (0)
#define GAME_SPEED_MIN  (50)

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
HWND hWnd;                                      // 当前窗口
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
//BOOL  GanmeRunFlag = FALSE;
//dir_t GameTankDir = DIR_MAX;
//int32 GameTankFire = FALSE;
//int32 GameAutoFire = FALSE;
//int32 GameSpeed = 100;

game_ctrl_t GAME_CTRL = { 0 };

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Readme(HWND, UINT, WPARAM, LPARAM);
DWORD   WINAPI      TankRun(LPVOID lpProgram);
void                DebugOpen(void);
void                DebugClose(void);
void                GameCtrlInit(void);
void                GamePlaySound(int32 sid);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。
    HACCEL hAccelTable;
    MSG msg;
    DWORD   dwTankRunTId;
    HANDLE  hTankRunT;

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_GAMETANK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    DebugOpen();

    gdi_init(hWnd);//显示初始化
    gdi_clear(GDI_RGB_BACKGROUND);//显示初始化
    GameCtrlInit();
    tank_init();
    //GamePlaySound(IDR_WAVE_BACKG);

    hTankRunT = CreateThread(NULL, 0, TankRun, NULL, 0, &dwTankRunTId);
    //创建线程失败
    if (hTankRunT == NULL)
    {
        ExitProcess(0);//主程序退出
    }

    //快捷键
    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMETANK));

    // 主消息循环: 
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            //TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    HBRUSH hWindowBrush;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;                                       //如果大小改变了重绘窗口
    wcex.lpfnWndProc = WndProc;                                                 //窗口消息处理函数
    wcex.cbClsExtra = 0;                                                        //无附加窗口类内存
    wcex.cbWndExtra = 0;                                                        //无附加窗口内存
    wcex.hInstance = hInstance;                                                 //应用程序实例
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMETANK));            //图标
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);                                 //鼠标指针
#if 0
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);                            //背景画刷(默认白色)
#else
    hWindowBrush = CreateSolidBrush((COLORREF)GDI_RGB_BACKGROUND);              //背景画刷(自定义)
    wcex.hbrBackground = hWindowBrush;
#endif
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAMETANK);                         //菜单资源
    wcex.lpszClassName = szWindowClass;                                         //窗口类名
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));        //小图标

    return RegisterClassEx(&wcex);                                             //注册窗口类
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    //自定义窗口样式
    DWORD dwStyle =
        WS_OVERLAPPEDWINDOW |
        WS_CLIPCHILDREN |
        WS_CLIPSIBLINGS |
        WS_POPUPWINDOW;
    dwStyle &= (~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX));//不能改变大小

    //自定义窗口大小
    RECT wndRect;
    wndRect.left = 0;
    wndRect.top = 0;
#if 0
    wndRect.right = MAX_WND_WIDTH + GetSystemMetrics(SM_CXFRAME);
    wndRect.bottom = MAX_WND_HEIGHT + GetSystemMetrics(SM_CYCAPTION) +
        GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYMENU);
    AdjustWindowRect(&wndRect, WS_CAPTION, FALSE);
#else
    wndRect.right = MAX_WND_WIDTH;
    wndRect.bottom = MAX_WND_HEIGHT;
    AdjustWindowRect(&wndRect, dwStyle, TRUE);//指定窗口客户区大小,窗口风格,窗户包含菜单
#endif

    /*HWND*/hWnd = CreateWindow(
        szWindowClass,                  //窗口类名
        szTitle,                        //窗口名
        /*WS_OVERLAPPEDWINDOW*/dwStyle, //窗口样式
        CW_USEDEFAULT,                  //水平位置，默认
        CW_USEDEFAULT,                  //垂直位置，默认
        wndRect.right - wndRect.left,   //宽
        wndRect.bottom - wndRect.top,   //高
        NULL,                           //无父窗口
        (HMENU)/*NULL*/LoadMenu(hInst, MAKEINTRESOURCE(IDC_GAMETANK)), //菜单
        hInstance,                      //应用程序实例
        NULL);                          //无窗口创建数据

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    PAINTSTRUCT ps;
    HDC hdc;
    switch (message)
    {
        //case WM_CREATE:
        //    break;
    case WM_COMMAND:
    {
        wmId = LOWORD(wParam);
        // 分析菜单选择: 
        switch (wmId)
        {
        case IDM_ABOUT: //关于
            GAME_CTRL.run = FALSE;//点击对话框的时候暂停
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_README: //说明
            GAME_CTRL.run = FALSE;//点击对话框的时候暂停
            DialogBox(hInst, MAKEINTRESOURCE(IDD_READMEBOX), hWnd, Readme);
            break;
        case IDM_EXIT://退出
            DestroyWindow(hWnd);
            break;
        case IDM_STOP://暂停
            GAME_CTRL.run = FALSE;
            break;
        case IDM_CONTINUE://继续
            GAME_CTRL.run = TRUE;
            break;
        case IDM_ADDSPEED://加速
            if (GAME_CTRL.speed >= GAME_SPEED_MAX)
            {
                GAME_CTRL.speed -= 10;
            }
            break;
        case IDM_SUBSPEED://减速
            if (GAME_CTRL.speed <= GAME_SPEED_MIN)
            {
                GAME_CTRL.speed += 10;
            }
            break;
        case IDM_AUTOFIRE://自动开火
            GAME_CTRL.auto_fire = TRUE;
            break;
        case IDM_HANDFIRE://手动开火
            GAME_CTRL.auto_fire = FALSE;
            break;
        case IDM_TANKFIRE://开火
            GAME_CTRL.fire = TRUE;
            break;
        case IDM_DIFFICULT_A:
            GAME_CTRL.difficult = DIFFICULT_A;
            break;
        case IDM_DIFFICULT_B:
            GAME_CTRL.difficult = DIFFICULT_B;
            break;
        case IDM_DIFFICULT_C:
            GAME_CTRL.difficult = DIFFICULT_C;
            break;
        case IDM_DIFFICULT_D:
            GAME_CTRL.difficult = DIFFICULT_D;
            break;
        case IDM_DEBUG_ON:
            GAME_CTRL.debug = TRUE;
            break;
        case IDM_DEBUG_OFF:
            GAME_CTRL.debug = FALSE;
            break;
        case IDM_SUPER_ON:
            GAME_CTRL.super = TRUE;
            break;
        case IDM_SUPER_OFF:
            GAME_CTRL.super = FALSE;
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        //gdi_textout(0, 0, 20, GDI_RGB_RED, GDI_TMODE_BOLD | GDI_TMODE_ITALIC | GDI_TMODE_UNDLINE, TEXT("Hello World!"));
        gdi_update();
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        gdi_dinit();
        GamePlaySound(0);
        DebugClose();
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP://上
            GAME_CTRL.dir = DIR_UP;
            break;
        case VK_DOWN://下
            GAME_CTRL.dir = DIR_DOWN;
            break;
        case VK_LEFT://左
            GAME_CTRL.dir = DIR_LEFT;
            break;
        case VK_RIGHT://右
            GAME_CTRL.dir = DIR_RIGHT;
            break;
        case VK_RETURN://回车键开火
            GAME_CTRL.fire = TRUE;
            break;
        case VK_F1://加速
            if (GAME_CTRL.speed > GAME_SPEED_MAX)
            {
                GAME_CTRL.speed -= 10;
            }
            break;
        case VK_F2://减速
            if (GAME_CTRL.speed < GAME_SPEED_MIN)
            {
                GAME_CTRL.speed += 10;
            }
            break;
        case VK_F3://自动开火
            GAME_CTRL.auto_fire = TRUE;
            break;
        case VK_F4://手动开火
            GAME_CTRL.auto_fire = FALSE;
            break;
#if 0
        case VK_F5://
            GAME_CTRL.difficult = DIFFICULT_A;
            break;
        case VK_F6://
            GAME_CTRL.difficult = DIFFICULT_B;
            break;
        case VK_F7://
            GAME_CTRL.difficult = DIFFICULT_C;
            break;
        case VK_F8://
            GAME_CTRL.difficult = DIFFICULT_D;
            break;
#endif
        case VK_F5://
            GAME_CTRL.debug = TRUE;
            break;
        case VK_F6://
            GAME_CTRL.debug = FALSE;
            break;
        case VK_F7://
            GAME_CTRL.super = TRUE;
            break;
        case VK_F8://
            GAME_CTRL.super = FALSE;
            break;
        case VK_SPACE://暂停/开始
            if (GAME_CTRL.run == FALSE)
            {
                GAME_CTRL.run = TRUE;
            }
            else
            {
                GAME_CTRL.run = FALSE;
            }
            break;
        default:
            break;
        }
        break;
        //case WM_CHAR:
        //    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            GAME_CTRL.run = TRUE;//点击对话框的时候暂停
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Readme(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            GAME_CTRL.run = TRUE;//点击对话框的时候暂停
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//游戏运行线程
DWORD WINAPI TankRun(LPVOID lpProgram)
{
    while (TRUE)
    {
        if (GAME_CTRL.run)//暂停游戏
        {
            if (GAME_CTRL.auto_fire != FALSE)
            {
                // GameTankDir = ((i++) % DIR_MAX);
                GAME_CTRL.fire = TRUE;
            }

            if (tank_run(&(GAME_CTRL.dir), &(GAME_CTRL.fire)/*, GAME_CTRL.difficult*/, GAME_CTRL.super, GAME_CTRL.debug) != RTN_OK)
            {
                //DEBUG_LOG("ERR");
                MessageBox(/*NULL*/hWnd, TEXT("继续战斗吧!"), TEXT("你输了!"), MB_OK);
                GameCtrlInit();
                tank_init();
            }
            Sleep(1 + GAME_CTRL.speed);
        }
    }
    return 0;
}

void GameCtrlInit(void)
{
    GAME_CTRL.run = TRUE;
    GAME_CTRL.dir = DIR_MAX;
    GAME_CTRL.fire = FALSE;
    GAME_CTRL.auto_fire = FALSE;
    GAME_CTRL.speed = GAME_SPEED_MIN;
    GAME_CTRL.difficult = DIFFICULT_MIN;
    GAME_CTRL.debug = FALSE;
    GAME_CTRL.super = FALSE;
}

void GamePlaySound(int32 sid)
{
#if 0
    if (sid)
    {//start
        PlaySound(MAKEINTRESOURCE(sid), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC | SND_LOOP);
    }
    else
    {//stop
        PlaySound(NULL, 0, 0);
}
#endif
}

void DebugOpen(void)
{
#ifdef __DEBUG__
    FILE *stream;
    AllocConsole();
    SetConsoleTitle(_T("Tank_Debug"));
    freopen_s(&stream, "CONOUT$", "w+t", stdout);
    freopen_s(&stream, "CONIN$", "r+t", stdin);
#endif
    }

void DebugClose(void)
{
#ifdef __DEBUG__
    FreeConsole();
#endif
}