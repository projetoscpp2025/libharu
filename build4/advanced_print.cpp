#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <cstdlib>

#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

// ===== GDI+
ULONG_PTR token;

void InitGDI() {
    GdiplusStartupInput input;
    GdiplusStartup(&token, &input, NULL);
}

// ===== PNG ENCODER =====
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j) {
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

// ===== CAPTURA =====
HBITMAP CapturarArea(int x, int y, int w, int h) {
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    SelectObject(hDC, hBitmap);

    BitBlt(hDC, 0, 0, w, h, hScreen, x, y, SRCCOPY);

    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);

    return hBitmap;
}

// ===== SALVAR PNG =====
void SalvarPNG(HBITMAP hBitmap) {
    Bitmap bmp(hBitmap, NULL);

    CLSID clsid;
    GetEncoderClsid(L"image/png", &clsid);

    bmp.Save(L"print.png", &clsid, NULL);
}

// ===== CLIPBOARD =====
void CopiarClipboard(HBITMAP hBitmap) {
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();
}

// ===== VARIÁVEIS =====
POINT inicio = {0,0}, fim = {0,0};
bool selecionando = false;

// ===== CALLBACK =====
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch(msg) {

        case WM_SETCURSOR:
            SetCursor(LoadCursor(NULL, IDC_CROSS));
            return TRUE;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_LBUTTONDOWN:
            inicio.x = LOWORD(lParam);
            inicio.y = HIWORD(lParam);
            fim = inicio;
            selecionando = true;
            break;

        case WM_MOUSEMOVE:
            if(selecionando) {
                fim.x = LOWORD(lParam);
                fim.y = HIWORD(lParam);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_LBUTTONUP:
        {
            selecionando = false;

            int x = std::min(inicio.x, fim.x);
            int y = std::min(inicio.y, fim.y);
            int w = std::abs(fim.x - inicio.x);
            int h = std::abs(fim.y - inicio.y);

            if(w > 10 && h > 10) {
                HBITMAP bmp = CapturarArea(x, y, w, h);
                SalvarPNG(bmp);
                CopiarClipboard(bmp);
                DeleteObject(bmp);
            }

            DestroyWindow(hwnd);
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            // fundo escuro semi-transparente
            HBRUSH brush = CreateSolidBrush(RGB(0,0,0));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            if(selecionando) {

                int x = std::min(inicio.x, fim.x);
                int y = std::min(inicio.y, fim.y);
                int w = std::abs(fim.x - inicio.x);
                int h = std::abs(fim.y - inicio.y);

                // região total
                HRGN full = CreateRectRgn(0, 0, rect.right, rect.bottom);

                // região seleção
                HRGN cut = CreateRectRgn(x, y, x+w, y+h);

                // remove área da seleção
                CombineRgn(full, full, cut, RGN_DIFF);

                // aplica recorte
                SelectClipRgn(hdc, full);

                HBRUSH brush2 = CreateSolidBrush(RGB(0,0,0));
                FillRect(hdc, &rect, brush2);
                DeleteObject(brush2);

                SelectClipRgn(hdc, NULL);

                // borda da seleção
                HPEN pen = CreatePen(PS_SOLID, 2, RGB(0,255,0));
                SelectObject(hdc, pen);
                Rectangle(hdc, x, y, x+w, y+h);
                DeleteObject(pen);

                DeleteObject(full);
                DeleteObject(cut);
            }

            EndPaint(hwnd, &ps);
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ===== MAIN =====
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    InitGDI();

    const char CLASS_NAME[] = "OverlayCapture";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        CLASS_NAME,
        "",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    // transparência (150/255)
    SetLayeredWindowAttributes(hwnd, 0, 150, LWA_ALPHA);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(token);
    return 0;
}