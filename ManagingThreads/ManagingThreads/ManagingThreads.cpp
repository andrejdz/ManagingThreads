#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <strsafe.h>
#include <math.h>
#include "resource.h"

#define PI 3.14159265359

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgProc_About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_InfoThreads(HWND, UINT, WPARAM, LPARAM);

BOOL WndProc_OnCreate(HWND, LPCREATESTRUCT);
void WndProc_OnCommand(HWND, int, HWND, UINT);
void WndProc_OnDestroy(HWND);
BOOL WndProc_OnEraseBkgnd(HWND, HDC);

void DialogProc_OnCommand(HWND, int, HWND, UINT);
BOOL DialogProc_OnInitDialog(HWND, HWND, LPARAM);

BOOL DialogInfoThreads_OnInitDialog(HWND, HWND, LPARAM);
void DialogInfoThreads_OnCommand(HWND, int, HWND, UINT);

void ErrorExit(LPTSTR);
BOOL ErrorExitBool(LPTSTR);

LPVOID AllocMem(size_t);
void FreeAllocMem(LPVOID);

DWORD WINAPI RunningStr(LPVOID);
DWORD WINAPI RunningStr2(LPVOID);
DWORD WINAPI CreateShapes(LPVOID);

LPTSTR g_lpszClassName = TEXT("ManagingThreads");
LPTSTR g_lpszApplicationTitle = TEXT("Developer: Dyagel Andrew");
LPTSTR g_lpcszMessagePaint1;
LPTSTR g_lpcszMessagePaint2;

HANDLE g_threadHandle[4];
DWORD g_threadID[4];

HANDLE g_hMutex;
BOOL g_bMutex = FALSE;

struct THREAD_PARAM
{
    DWORD number;
    UINT x;
    UINT y;
    HWND hwnd;
};

int APIENTRY _tWinMain(HINSTANCE This,
                       HINSTANCE hPrevInstance,
                       LPTSTR lpszCmdLine,
                       int mode)
{
    HWND hWnd;
    HWND hDlgMode = NULL;
    MSG msg;
    WNDCLASSEX wc;

    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = This;
    wc.lpszClassName = g_lpszClassName;
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIcon(This, MAKEINTRESOURCE(IDI_ICON_LOGO));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_CUSTOM);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIconSm = NULL;

    if(!RegisterClassEx(&wc))
    {
        return ErrorExitBool(TEXT("RegisterClassEx()"));
    }

    hWnd = CreateWindowEx(NULL,
                          g_lpszClassName,
                          g_lpszApplicationTitle,
                          WS_OVERLAPPEDWINDOW | WS_CAPTION,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          This,
                          NULL);
    if(!hWnd)
    {
        return ErrorExitBool(TEXT("CreateWindowEx()"));
    }

    ShowWindow(hWnd, mode);
    UpdateWindow(hWnd);

    HACCEL hAccel;
    hAccel = LoadAccelerators(This, MAKEINTRESOURCE(IDR_ACCELERATOR1));
    if(!hAccel)
    {
        return ErrorExitBool(TEXT("LoadAccelerators()"));
    }

    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(hDlgMode == 0 || !IsDialogMessage(hDlgMode, &msg))
        {
            if(!TranslateAccelerator(hWnd, hAccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    switch(msg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, WndProc_OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, WndProc_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, WndProc_OnDestroy);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, WndProc_OnEraseBkgnd);
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK DlgProc_About(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, DialogProc_OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, DialogProc_OnCommand);
    }
    return FALSE;
}

INT_PTR CALLBACK DlgProc_InfoThreads(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, DialogInfoThreads_OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, DialogInfoThreads_OnCommand);
    }
    return FALSE;
}

BOOL WndProc_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_threadHandle[0] = GetCurrentThread();
    g_threadID[0] = GetCurrentThreadId();

    g_hMutex = CreateMutex(NULL, FALSE, TEXT("MyMutex"));
    if(g_hMutex == NULL)
    {
        return ErrorExitBool(TEXT("CreateMutex()"));
    }

    return TRUE;
}

void WndProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    int priority;
    THREAD_PARAM threadParam1 = { 1, 10, 25, hwnd };;
    THREAD_PARAM threadParam2 = { 2, 10, 75, hwnd };


    if(codeNotify == 1)
    {
        switch(id)
        {
            case ID_ACCELERATOR_EXIT:
                SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
                break;
        }
    }
    else
    {
        switch(id)
        {
            case ID_THREAD1_CREATETHREAD:
                g_threadHandle[1] = CreateThread(NULL, 0, RunningStr, &threadParam1, 0, &g_threadID[1]);
                if(!g_threadHandle[1])
                {
                    ErrorExit(TEXT("CreateThread()"));
                }
                Sleep(5);

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_SUSPENDTHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_RESUMETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_CREATETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_CREATEIDLETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD1, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                break;
            case ID_THREAD1_CREATEIDLETHREAD:
                g_threadHandle[1] = CreateThread(NULL, 0, RunningStr, &threadParam1, CREATE_SUSPENDED, &g_threadID[1]);
                if(!g_threadHandle[1])
                {
                    ErrorExit(TEXT("CreateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_CREATETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_SUSPENDTHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_RESUMETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD1, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                break;
            case ID_THREAD1_SUSPENDTHREAD:
                if(SuspendThread(g_threadHandle[1]) == -1)
                {
                    ErrorExit(TEXT("SuspendThread()"));
                }
                break;
            case ID_THREAD1_RESUMETHREAD:
                if(ResumeThread(g_threadHandle[1]) == -1)
                {
                    ErrorExit(TEXT("ResumeThread()"));
                }
                Sleep(5);
                break;
            case ID_THREAD1_DESTROYTHREAD:
                if(TerminateThread(g_threadHandle[1], 0) == 0)
                {
                    ErrorExit(TEXT("TerminateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_SUSPENDTHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_RESUMETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_CREATETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD1_CREATEIDLETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD1, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(!CloseHandle(g_threadHandle[1]))
                {
                    ErrorExit(TEXT("CloseHandle()"));
                }

                break;
            case ID_THREAD1_INCREASEPRIORITY:
                priority = GetThreadPriority(g_threadHandle[1]);

                if(priority == THREAD_PRIORITY_ERROR_RETURN)
                {
                    ErrorExit(TEXT("GetThreadPriority()"));
                }

                priority++;
                if(priority > 2 && priority < 15)
                {
                    priority = 15;
                }
                if(priority < -2 && priority > -15)
                {
                    priority = -2;
                }

                if(priority >= -15 && priority <= 15)
                {
                    SetThreadPriority(g_threadHandle[1], priority);
                }
                break;
            case ID_THREAD1_DECREASEPRIORITY:
                priority = GetThreadPriority(g_threadHandle[1]);

                if(priority == THREAD_PRIORITY_ERROR_RETURN)
                {
                    ErrorExit(TEXT("GetThreadPriority()"));
                }

                priority--;
                if(priority > 2 && priority < 15)
                {
                    priority = 2;
                }
                if(priority < -2 && priority > -15)
                {
                    priority = -15;
                }

                if(priority >= -15 && priority <= 15)
                {
                    SetThreadPriority(g_threadHandle[1], priority);
                }
                break;
            case ID_THREAD2_CREATETHREAD:
                g_threadHandle[2] = CreateThread(NULL, 0, RunningStr, &threadParam2, 0, &g_threadID[2]);
                if(!g_threadHandle[2])
                {
                    ErrorExit(TEXT("CreateThread()"));
                }
                Sleep(5);

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_SUSPENDTHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_RESUMETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_CREATETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_CREATEIDLETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD2, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                break;
            case ID_THREAD2_CREATEIDLETHREAD:
                g_threadHandle[2] = CreateThread(NULL, 0, RunningStr, &threadParam2, CREATE_SUSPENDED, &g_threadID[2]);
                if(!g_threadHandle[2])
                {
                    ErrorExit(TEXT("CreateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_CREATETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_SUSPENDTHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_RESUMETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD2, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                break;
            case ID_THREAD2_SUSPENDTHREAD:
                if(SuspendThread(g_threadHandle[2]) == -1)
                {
                    ErrorExit(TEXT("SuspendThread()"));
                }
                break;
            case ID_THREAD2_RESUMETHREAD:
                if(ResumeThread(g_threadHandle[2]) == -1)
                {
                    ErrorExit(TEXT("ResumeThread()"));
                }
                Sleep(5);
                break;
            case ID_THREAD2_DESTROYTHREAD:
                if(TerminateThread(g_threadHandle[2], 0) == 0)
                {
                    ErrorExit(TEXT("TerminateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_SUSPENDTHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_RESUMETHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_CREATETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREAD2_CREATEIDLETHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_INFOABOUTTHREADS_THREAD2, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(!CloseHandle(g_threadHandle[2]))
                {
                    ErrorExit(TEXT("CloseHandle()"));
                }

                break;
            case ID_THREAD2_INCREASEPRIORITY:
                priority = GetThreadPriority(g_threadHandle[2]);

                if(priority == THREAD_PRIORITY_ERROR_RETURN)
                {
                    ErrorExit(TEXT("GetThreadPriority()"));
                }

                priority++;
                if(priority > 2 && priority < 15)
                {
                    priority = 15;
                }
                if(priority < -2 && priority > -15)
                {
                    priority = -2;
                }

                if(priority >= -15 && priority <= 15)
                {
                    SetThreadPriority(g_threadHandle[2], priority);
                }
                break;
            case ID_THREAD2_DECREASEPRIORITY:
                priority = GetThreadPriority(g_threadHandle[2]);

                if(priority == THREAD_PRIORITY_ERROR_RETURN)
                {
                    ErrorExit(TEXT("GetThreadPriority()"));
                }

                priority--;
                if(priority > 2 && priority < 15)
                {
                    priority = 2;
                }

                if(priority < -2 && priority > -15)
                {
                    priority = -15;
                }

                if(priority >= -15 && priority <= 15)
                {
                    SetThreadPriority(g_threadHandle[2], priority);
                }
                break;
            case ID_THREADWITHANIMATION_CREATETHREAD:
                g_threadHandle[3] = CreateThread(NULL, 0, CreateShapes, hwnd, 0, &g_threadID[3]);
                if(!g_threadHandle[3])
                {
                    ErrorExit(TEXT("CreateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREADWITHANIMATION_DESTROYTHREAD, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }
                break;
            case ID_THREADWITHANIMATION_DESTROYTHREAD:
                if(TerminateThread(g_threadHandle[3], 0) == 0)
                {
                    ErrorExit(TEXT("TerminateThread()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_THREADWITHANIMATION_DESTROYTHREAD, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }
                break;
            case ID_SYNCHRONISATION_ENABLE:
                g_bMutex = TRUE;

                if(EnableMenuItem(GetMenu(hwnd), ID_SYNCHRONISATION_ENABLE, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_SYNCHRONISATION_DISABLE, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }
                break;
            case ID_SYNCHRONISATION_DISABLE:
                g_bMutex = FALSE;

                if(EnableMenuItem(GetMenu(hwnd), ID_SYNCHRONISATION_ENABLE, MF_ENABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }

                if(EnableMenuItem(GetMenu(hwnd), ID_SYNCHRONISATION_DISABLE, MF_DISABLED) == -1)
                {
                    ErrorExit(TEXT("EnableMenuItem()"));
                }
                break;
            case ID_INFOABOUTTHREADS_PRIMARYTHREAD:
                DialogBoxParam((HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG4),
                               hwnd, (DLGPROC)DlgProc_InfoThreads, (LPARAM)0);
                break;
            case ID_INFOABOUTTHREADS_THREAD1:
                DialogBoxParam((HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG4),
                               hwnd, (DLGPROC)DlgProc_InfoThreads, (LPARAM)1);
                break;
            case ID_INFOABOUTTHREADS_THREAD2:
                DialogBoxParam((HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG4),
                               hwnd, (DLGPROC)DlgProc_InfoThreads, (LPARAM)2);
                break;
            case ID_HELP_ABOUT:
                DialogBox((HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG1),
                          hwnd, (DLGPROC)DlgProc_About);
                break;
            case IDCANCEL:
                DestroyWindow(hwnd);
                break;
        }
    }
    FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, DefWindowProc);
}

void WndProc_OnDestroy(HWND hwnd)
{
    FreeAllocMem(g_lpcszMessagePaint1);
    FreeAllocMem(g_lpcszMessagePaint2);

    PostQuitMessage(0);
    FORWARD_WM_DESTROY(hwnd, DefWindowProc);
}

void DialogProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
        case IDOK:
            EndDialog(hwnd, IDOK);
    }
    FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, DefWindowProc);
}

BOOL DialogProc_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPCTSTR lpszTextAbout = TEXT("Windows Application\nDeveloper: Dyagel A.Y.\nGroup: 60323-1\n");

    LPTSTR lpszTextBuffer = (LPTSTR)AllocMem(MAX_CHARS);
    LPTSTR lpszTimeBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    SYSTEMTIME st;

    GetLocalTime(&st);

    StringCchPrintf(lpszTimeBuffer, MAX_CHARS, TEXT("Date: %02d.%02d.%d\nTime: %02d:%02d:%02d"), st.wDay, st.wMonth, st.wYear,
                    st.wHour, st.wMinute, st.wSecond);
    StringCchCat(lpszTextBuffer, MAX_CHARS, lpszTextAbout);
    StringCchCat(lpszTextBuffer, MAX_CHARS, lpszTimeBuffer);

    SetWindowText(GetDlgItem(hwnd, IDC_STATIC1), lpszTextBuffer);

    FreeAllocMem(lpszTextBuffer);
    FreeAllocMem(lpszTimeBuffer);

    return TRUE;
}

BOOL DialogInfoThreads_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPTSTR lpszTextBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    StringCchPrintf(lpszTextBuffer, MAX_CHARS, TEXT("Handle: %u\r\nTid: %u"),
                    g_threadHandle[lParam], g_threadID[lParam]);

    if(!SetWindowText(GetDlgItem(hwnd, IDC_EDITID), lpszTextBuffer))
    {
        return ErrorExitBool(TEXT("SetWindowText()"));
    }

    FreeAllocMem(lpszTextBuffer);

    lpszTextBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    DWORD exitCode;
    if(!GetExitCodeThread((HANDLE)g_threadHandle[lParam], &exitCode))
    {
        return ErrorExitBool(TEXT("GetExitCodeThread()"));
    }

    if(exitCode == STILL_ACTIVE)
    {
        if(!SetWindowText(GetDlgItem(hwnd, IDC_EDITSTATE), TEXT("Still active")))
        {
            return ErrorExitBool(TEXT("SetWindowText()"));
        }
    }
    else
    {
        StringCchPrintf(lpszTextBuffer, MAX_CHARS, TEXT("%u"),
                        exitCode);

        if(!SetWindowText(GetDlgItem(hwnd, IDC_EDITSTATE), lpszTextBuffer))
        {
            return ErrorExitBool(TEXT("SetWindowText()"));
        }
    }

    FreeAllocMem(lpszTextBuffer);

    lpszTextBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    switch(GetThreadPriority(g_threadHandle[lParam]))
    {
        case THREAD_PRIORITY_ERROR_RETURN:
            return ErrorExitBool(TEXT("GetThreadPriority()"));
        case THREAD_PRIORITY_ABOVE_NORMAL:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Above normal"));
            break;
        case THREAD_PRIORITY_BELOW_NORMAL:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Below normal"));
            break;
        case THREAD_PRIORITY_HIGHEST:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Highest"));
            break;
        case THREAD_PRIORITY_IDLE:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Idle"));
            break;
        case THREAD_PRIORITY_LOWEST:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Lowest"));
            break;
        case THREAD_PRIORITY_NORMAL:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Normal"));
            break;
        case THREAD_PRIORITY_TIME_CRITICAL:
            StringCchCopy(lpszTextBuffer, MAX_CHARS, TEXT("Time critical"));
            break;
    }

    if(!SetWindowText(GetDlgItem(hwnd, IDC_EDITPRIORITY), lpszTextBuffer))
    {
        return ErrorExitBool(TEXT("SetWindowText()"));
    }

    FreeAllocMem(lpszTextBuffer);

    lpszTextBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    FILETIME ftCreationTime, ftlpCreationTime;
    FILETIME ftExitTime, ftlpExitTime;
    FILETIME ftKernelTime;
    FILETIME ftUserTime;

    if(!GetThreadTimes((HANDLE)g_threadHandle[lParam], &ftCreationTime,
                       &ftExitTime, &ftKernelTime, &ftUserTime))
    {
        return ErrorExitBool(TEXT("GetThreadTimes()"));
    }

    SYSTEMTIME stCreationTime;
    SYSTEMTIME stExitTime;

    if(!FileTimeToLocalFileTime(&ftCreationTime, &ftlpCreationTime))
    {
        return ErrorExitBool(TEXT("FileTimeToLocalFileTime()"));
    }

    if(!FileTimeToLocalFileTime(&ftExitTime, &ftlpExitTime))
    {
        return ErrorExitBool(TEXT("FileTimeToLocalFileTime()"));
    }

    if(!FileTimeToSystemTime(&ftlpCreationTime, &stCreationTime))
    {
        return ErrorExitBool(TEXT("FileTimeToSystemTime()"));
    }

    if(!FileTimeToSystemTime(&ftlpExitTime, &stExitTime))
    {
        return ErrorExitBool(TEXT("FileTimeToSystemTime()"));
    }

    SYSTEMTIME stCurrentTime;
    FILETIME ftCurrentTime;
    GetSystemTime(&stCurrentTime);
    if(!SystemTimeToFileTime(&stCurrentTime, &ftCurrentTime))
    {
        return ErrorExitBool(TEXT("SystemTimeToFileTime()"));
    }

    ULARGE_INTEGER uiCurrentTime;
    uiCurrentTime.HighPart = ftCurrentTime.dwHighDateTime;
    uiCurrentTime.LowPart = ftCurrentTime.dwLowDateTime;

    ULARGE_INTEGER uiCreationTime;
    uiCreationTime.HighPart = ftCreationTime.dwHighDateTime;
    uiCreationTime.LowPart = ftCreationTime.dwLowDateTime;

    ULARGE_INTEGER uiExitTime;
    uiExitTime.HighPart = ftExitTime.dwHighDateTime;
    uiExitTime.LowPart = ftExitTime.dwLowDateTime;

    DWORD dwLifeTime;
    if(exitCode == STILL_ACTIVE)
    {
        dwLifeTime = (uiCurrentTime.QuadPart - uiCreationTime.QuadPart) * 0.0001;
    }
    else
    {
        dwLifeTime = (uiExitTime.QuadPart - uiCreationTime.QuadPart) * 0.0001;
    }

    ULARGE_INTEGER uiKernelTime;
    uiKernelTime.HighPart = ftKernelTime.dwHighDateTime;
    uiKernelTime.LowPart = ftKernelTime.dwLowDateTime;

    DWORD dwKernelTime = uiKernelTime.QuadPart * 0.0001;

    ULARGE_INTEGER uiUserTime;
    uiUserTime.HighPart = ftUserTime.dwHighDateTime;
    uiUserTime.LowPart = ftUserTime.dwLowDateTime;

    DWORD dwUserTime = uiUserTime.QuadPart * 0.0001;

    DWORD dwIdleTime = dwLifeTime - (dwKernelTime + dwUserTime);

    LPTSTR lpszFormat = (LPTSTR)AllocMem(MAX_CHARS);
    StringCchCat(lpszFormat, MAX_CHARS, TEXT("Creation time: %02d.%02d.%d %02d:%02d:%02d\r\n"));
    StringCchCat(lpszFormat, MAX_CHARS, TEXT("Exit time: %02d.%02d.%d %02d:%02d:%02d\r\n"));
    StringCchCat(lpszFormat, MAX_CHARS, TEXT("Life time: %u ms\r\nKernel time: %u ms\r\n"));
    StringCchCat(lpszFormat, MAX_CHARS, TEXT("User time: %u ms\r\nIdle time: %u ms"));

    StringCchPrintf(lpszTextBuffer, MAX_CHARS, lpszFormat,
                    stCreationTime.wDay, stCreationTime.wMonth, stCreationTime.wYear,
                    stCreationTime.wHour, stCreationTime.wMinute, stCreationTime.wSecond,
                    stExitTime.wDay, stExitTime.wMonth, stExitTime.wYear,
                    stExitTime.wHour, stExitTime.wMinute, stExitTime.wSecond,
                    dwLifeTime, dwKernelTime, dwUserTime, dwIdleTime);

    if(!SetWindowText(GetDlgItem(hwnd, IDC_EDITSUM), lpszTextBuffer))
    {
        return ErrorExitBool(TEXT("SetWindowText()"));
    }

    FreeAllocMem(lpszFormat);
    FreeAllocMem(lpszTextBuffer);

    return TRUE;
}

void DialogInfoThreads_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
        case IDOK:
            EndDialog(hwnd, IDOK);
    }
    FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, DefWindowProc);
}

void ErrorExit(LPTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_ICONERROR);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

BOOL ErrorExitBool(LPTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_ICONERROR);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    return FALSE;
}

LPVOID AllocMem(size_t bufferSize)
{
    LPVOID pointer = (LPVOID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);;
    if(pointer == NULL)
    {
        ErrorExit(TEXT("HeapAlloc()"));
    }

    return pointer;
}

void FreeAllocMem(LPVOID pointer)
{
    if(HeapFree(GetProcessHeap(), NULL, pointer) == 0)
    {
        ErrorExit(TEXT("HeapFree()"));
    }
}

DWORD WINAPI RunningStr(LPVOID lpParam)
{
    THREAD_PARAM *threadParam;
    threadParam = (THREAD_PARAM *)lpParam;

    RECT rect;
    HDC hdc;
    LPTSTR lpszBuffer;
    int iBeginningIndex;
    size_t length;
    int cRuns;
    int N = 1;
    BOOL bLocal = FALSE;

    lpszBuffer = (LPTSTR)AllocMem(MAX_CHARS);

    if(FAILED(StringCchPrintf(lpszBuffer, MAX_CHARS,
                              TEXT("Secondary thread %d have been created by Dyagel!"),
                              threadParam->number)))
    {
        ErrorExit(TEXT("StringCchPrintf()"));
    }

    if(FAILED(StringCchLength(lpszBuffer, MAX_CHARS, &length)))
    {
        ErrorExit(TEXT("StringCchLength()"));
    }

    iBeginningIndex = length;

    if(GetClientRect(threadParam->hwnd, &rect) == 0)
    {
        return 1;
    }

    rect.left = threadParam->x;
    rect.top = threadParam->y;

    hdc = GetDC(threadParam->hwnd);
    if(!hdc)
    {
        return 1;
    }

    while(TRUE)
    {
        if(g_bMutex)
        {
            bLocal = TRUE;
            WaitForSingleObject(g_hMutex, INFINITE);
        }

        cRuns = 0;
        while(cRuns < N)
        {
            for(size_t i = 0; i < length; i++)
            {
                if(iBeginningIndex == 0)
                {
                    iBeginningIndex = length;
                    cRuns++;
                }

                TCHAR c;
                c = lpszBuffer[length];

                for(size_t i = length; i > 0; i--)
                {
                    lpszBuffer[i] = lpszBuffer[i - 1];
                }
                lpszBuffer[0] = c;

                if(DrawText(hdc, lpszBuffer, MAX_CHARS,
                            &rect, DT_LEFT | DT_SINGLELINE) == 0)
                {
                    ErrorExit(TEXT("DrawText()"));
                }

                iBeginningIndex--;
                Sleep(200);
            }
        }
        if(bLocal)
        {
            bLocal = FALSE;
            ReleaseMutex(g_hMutex);
        }
    }

    FreeAllocMem(lpszBuffer);
    return 0;
}

DWORD WINAPI CreateShapes(LPVOID lpParam)
{
    HWND hwnd = (HWND)lpParam;
    HDC hdc = GetDC(hwnd);;
    XFORM xForm;
    HPEN hPen;
    HBRUSH hBrushRect;
    HBRUSH hBrushElCross;
    HBRUSH hBrushElDiagCross;
    RECT rectClient;

    hPen = CreatePen(PS_SOLID, 2, RGB(0, 128, 0));
    if(!hPen)
    {
        ErrorExit(TEXT("CreatePen()"));
    }

    if(!SelectObject(hdc, hPen))
    {
        ErrorExit(TEXT("SelectObject()"));
    }

    hBrushRect = CreateSolidBrush(RGB(0, 0, 128));
    if(!hBrushRect)
    {
        ErrorExit(TEXT("CreateSolidBrush()"));
    }

    hBrushElCross = CreateHatchBrush(HS_CROSS, RGB(0, 0, 128));
    if(!hBrushElCross)
    {
        ErrorExit(TEXT("CreateHatchBrush()"));
    }

    hBrushElDiagCross = CreateHatchBrush(HS_DIAGCROSS, RGB(0, 0, 128));
    if(!hBrushElDiagCross)
    {
        ErrorExit(TEXT("CreateHatchBrush()"));
    }

    if(!SetGraphicsMode(hdc, GM_ADVANCED))
    {
        ErrorExit(TEXT("SetGraphicsMode()"));
    }

    if(!GetClientRect(hwnd, &rectClient))
    {
        ErrorExit(TEXT("GetClientRect()"));
    }

    if(!SetWindowOrgEx(hdc, -rectClient.right / 2, -rectClient.bottom / 2, NULL))
    {
        ErrorExit(TEXT("SetWindowOrgEx()"));
    }

    xForm.eDx = (FLOAT) 0.0;
    xForm.eDy = (FLOAT) 0.0;

    for(FLOAT angle = 0.0;; angle += 1.0)
    {
        xForm.eM11 = (FLOAT)cos(angle * PI / 180);
        xForm.eM12 = (FLOAT)sin(angle * PI / 180);
        xForm.eM21 = (FLOAT)-sin(angle * PI / 180);
        xForm.eM22 = (FLOAT)cos(angle * PI / 180);

        if(!SetWorldTransform(hdc, &xForm))
        {
            ErrorExit(TEXT("SetWorldTransform()"));
        }

        for(size_t i = 0; i < 100000; i++);

        if(!SelectObject(hdc, hBrushRect))
        {
            ErrorExit(TEXT("SelectObject()"));
        }
        Rectangle(hdc, -30, -60, 30, 60);

        if(!SelectObject(hdc, hBrushElCross))
        {
            ErrorExit(TEXT("SelectObject()"));
        }
        Pie(hdc, -90, -60, 30, 60, -30, -60, -30, 60);

        if(!SelectObject(hdc, hBrushElDiagCross))
        {
            ErrorExit(TEXT("SelectObject()"));
        }
        Pie(hdc, -30, -60, 90, 60, 30, 60, 30, -60);

        Sleep(25);

        InvalidateRect(hwnd, NULL, TRUE);
    }

    DeleteObject(hPen);
    DeleteObject(hBrushRect);
    DeleteObject(hBrushElCross);
    DeleteObject(hBrushElDiagCross);

    ReleaseDC(hwnd, hdc);

    return 0;
}

BOOL WndProc_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rectClient;

    if(!GetClientRect(hwnd, &rectClient))
    {
        ErrorExitBool(TEXT("GetClientRect()"));
    }

    if(!FillRect(hdc, &rectClient, (HBRUSH)(COLOR_WINDOW + 1)))
    {
        ErrorExitBool(TEXT("FillRect()"));
    }

    return TRUE;
}