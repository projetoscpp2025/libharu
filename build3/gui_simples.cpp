#include <windows.h>
#include <iostream>
using namespace std;

// ===== SUA FUNÇÃO (igual você já fez) =====
void CapturarTela(const char* arquivo) {
    int largura = GetSystemMetrics(SM_CXSCREEN);
    int altura = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, largura, altura);

    SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, largura, altura, hScreen, 0, 0, SRCCOPY);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = largura;
    bi.biHeight = altura;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((largura * bi.biBitCount + 31) / 32) * 4 * altura;

    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    GetDIBits(hDC, hBitmap, 0, altura, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFile(arquivo, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwSizeOfDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    DWORD dwBytesWritten;

    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeOfDIB;
    bmfHeader.bfType = 0x4D42;

    WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    CloseHandle(hFile);

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
}

// ===== EVENTO BOTÃO =====
void FazerPrint() {
    CapturarTela("print.bmp");
    MessageBox(NULL, "Print salvo como print.bmp!", "Sucesso", MB_OK | MB_ICONINFORMATION);
}

// ===== CALLBACK =====
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {

        case WM_CREATE:
            CreateWindow("BUTTON", "Capturar Tela",
                WS_VISIBLE | WS_CHILD,
                80, 50, 180, 40,
                hwnd, (HMENU)1, NULL, NULL);

            CreateWindow("BUTTON", "Sair",
                WS_VISIBLE | WS_CHILD,
                80, 110, 180, 40,
                hwnd, (HMENU)2, NULL, NULL);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 1:
                    FazerPrint();
                    break;
                case 2:
                    PostQuitMessage(0);
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ===== MAIN GUI =====
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    const char CLASS_NAME[] = "PrintSimples";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Capturador de Tela",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 250,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}