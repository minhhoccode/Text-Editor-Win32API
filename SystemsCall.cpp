#include <windows.h>
#include <tchar.h>
#include "id.hpp"
#include <iostream>

HWND hMainWindow;
static OPENFILENAME ofn;
static WCHAR link[260] = { 0 };
HINSTANCE hInstance;
static HWND box;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hWnd);
void OpenFile(HWND hWnd);
void SaveFile(HWND hWnd);
void about();

int main()
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = NULL;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = _T("SystemCall Application");
    wcex.hIconSm = NULL;
    RegisterClassEx(&wcex);

    HWND hMainWindow = CreateWindow(
        _T("SystemCall Application"),
        _T("SystemCall Application"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0, 800, 500,
        NULL, NULL, NULL, NULL);
    if (!hMainWindow)
    {
        return 0;
    }
    ShowWindow(hMainWindow, SW_SHOW);
    UpdateWindow(hMainWindow);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SYSTEMSCALL));
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            if (msg.message == WM_KEYDOWN)
                SendMessage(hMainWindow, WM_KEYDOWN, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ofn.lpstrFile = link;

    switch (message)
    {
    case WM_CREATE:
    {
        hMainWindow = hWnd;
        box = CreateWindow(_T("EDIT"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL,
            0, 0, 800, 500, hWnd, NULL, hInstance, NULL);
        AddMenus(hWnd);
    }
    break;
    case WM_SIZE:
        MoveWindow(box, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);
        break;
    case WM_KEYDOWN:
        if (GetKeyState(VK_CONTROL))
        {
            switch (wParam)
            {
            case 'A':
                SendMessage(box, EM_SETSEL, 0, -1);
                break;
            case 'O':
                OpenFile(hWnd);
                break;
            case 'S':
                SaveFile(hWnd);
                break;
            }
        }
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case ID_EDIT_COPY:
            SendMessage(box, WM_COPY, 0, 0);
            break;
        case ID_EDIT_CUT:
            SendMessage(box, WM_CUT, 0, 0);
            break;
        case ID_EDIT_PASTE:
            SendMessage(box, WM_PASTE, 0, 0);
            break;
        case ID_EDIT_SELECTALL:
            SendMessage(box, EM_SETSEL, 0, -1);
            break;
        case ID_EDIT_UNDO:
            SendMessage(box, WM_UNDO, 0, 0);
            break;
        case ID_FILE_OPEN:
            OpenFile(hWnd);
            break;
        case ID_FILE_SAVE:
            SaveFile(hWnd);
            break;
        case ID_ABOUT:
            about();
            break;
        }
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void AddMenus(HWND hWnd)
{
    HMENU hMenu, hSubMenu;
    hMenu = CreateMenu();
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_EDIT_COPY, _T("Copy"));
    AppendMenu(hSubMenu, MF_STRING, ID_EDIT_CUT, _T("Cut"));
    AppendMenu(hSubMenu, MF_STRING, ID_EDIT_PASTE, _T("Paste"));
    AppendMenu(hSubMenu, MF_STRING, ID_EDIT_SELECTALL, _T("Select All"));
    AppendMenu(hSubMenu, MF_STRING, ID_EDIT_UNDO, _T("Undo"));
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, _T("Edit"));
    AppendMenu(hMenu, MF_STRING, ID_FILE_OPEN, _T("Open"));
    AppendMenu(hMenu, MF_STRING, ID_FILE_SAVE, _T("Save"));
    AppendMenu(hMenu, MF_STRING, ID_ABOUT, _T("About"));
    SetMenu(hWnd, hMenu);
}
void OpenFile(HWND hWnd)
{
    SetWindowText(hWnd, L"");
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.nMaxFile = sizeof(link);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn))
    {
        HANDLE hFile = CreateFile(ofn.lpstrFile,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(NULL, L"Cannot open file", L"Error", MB_OK);
            return;
        }
        DWORD dwFileSize = GetFileSize(hFile, NULL);
        if (dwFileSize == INVALID_FILE_SIZE)
        {
            MessageBox(NULL, L"Cannot get file size", L"Error", MB_OK);
            return;
        }
        char* buffer = new char[dwFileSize + 1];
        DWORD dwBytesRead;
        if (!ReadFile(hFile, buffer, dwFileSize, &dwBytesRead, NULL))
        {
            MessageBox(NULL, L"Cannot read file", L"Error", MB_OK);
            return;
        }
        buffer[dwFileSize] = 0;
        SetWindowTextA(box, buffer);
        delete[] buffer;
        CloseHandle(hFile);
    }
}
void SaveFile(HWND hWnd)
{
    if (link[0] != 0)
    {
        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(NULL, _T("Cannot create file"), _T("Error"), MB_OK);
            return;
        }
        DWORD dwBytesWritten = 0;
        int nLength = GetWindowTextLength(box);
        char* buffer = new char[nLength + 1];
        GetWindowTextA(box, buffer, nLength + 1);
        WriteFile(hFile, buffer, nLength, &dwBytesWritten, NULL);
        CloseHandle(hFile);
        MessageBox(NULL, _T("Save file successfully"), _T("Save"), MB_OK);
        delete[] buffer;
    }
    else
    {
        long size = GetWindowTextLength(box);
        char* buffer = new char[size + 1];
        GetWindowTextA(box, buffer, size + 1);
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.nMaxFile = 260;
        ofn.lpstrFilter = L"Text document (*.txt) \0*.txt\0All file (*.*)\0*.*\0";
        ofn.nFilterIndex = 2;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetSaveFileName(&ofn) == TRUE)
        {
            HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            DWORD dwBytesWritten = 0;
            WriteFile(hFile, buffer, size, &dwBytesWritten, NULL);
            CloseHandle(hFile);
        }
    }
}
void about()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buffer[100];
    sprintf_s(buffer, "Basic Text Editor \nCurrent Date: %d/%d/%d Time: %d:%d:%d \nProcessID: %d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, GetCurrentProcessId());
    MessageBoxA(NULL, buffer, "About", MB_OK);
}
