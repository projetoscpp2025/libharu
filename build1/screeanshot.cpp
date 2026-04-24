#include <windows.h>
#include <iostream>
using namespace std;

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

int main() {
    CapturarTela("screeanshot.bmp");
    cout << "screeanshot salvo como screeanshot.bmp!" << endl;
    return 0;
}