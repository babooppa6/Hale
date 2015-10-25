#include <QEvent>
#include <QDebug>
#include <QtWin>

#include "frameless.h"

#if defined(Q_OS_WIN)

#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "dwmapi.lib")
#include <dwmapi.h>

#pragma comment(lib, "User32.lib")

// https://github.com/nishp/qt-utilities/blob/master/NcFramelessHelper/src/NcFramelessHelper.cpp

namespace {
static void
frameless_set(HWND hwnd)
{
    DWMNCRENDERINGPOLICY val = DWMNCRP_ENABLED;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &val, sizeof(DWMNCRENDERINGPOLICY));
    // This will remove the glass, but will keep the shadow.
    // MARGINS m = {1, 1, 1, 1};
    MARGINS m = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &m);

    // This will make syncing with DWM better (though I'm not sure what exactly is going on).
    // https://www.opengl.org/discussion_boards/showthread.php/164119-Making-composite-managers-go-along-with-OpenGL
    // Also look here (search for "fps.uiNumerator")
    // https://github.com/treblewinner/FB-Alpha/blob/master/src/burner/win32/dwmapi_core.cpp
    // More links:
    // - https://forums.geforce.com/default/topic/536046/geforce-drivers/windows-7-animation-lag/post/4001769/#4001769
//    DWM_PRESENT_PARAMETERS dpp;
//    memset(&dpp, 0, sizeof(DWM_PRESENT_PARAMETERS));
//    dpp.cbSize = sizeof(dpp);
//    dpp.fQueue = TRUE;
//    dpp.cBuffer = 2;
//    dpp.fUseSourceRate = FALSE;
//    dpp.cRefreshesPerFrame = 1;
//    dpp.eSampling = DWM_SOURCE_FRAME_SAMPLING_POINT;
//    ::DwmSetPresentParameters(hwnd, &dpp);

    // Reset styles.
//    SetWindowLong(hwnd, GWL_STYLE,
//        GetWindowLong(hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);

    SetWindowLong(hwnd, GWL_STYLE,
                  GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

//    RECT window_rect;
//    GetWindowRect(hwnd, &window_rect);

//    // Notify frame change.
//    SetWindowPos(hwnd,
//        NULL,
//        window_rect.left, window_rect.top,
//        window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
//        SWP_FRAMECHANGED);
}

static LRESULT
frameless_wm_nchittest(HWND hwnd, POINT mouse)
{
    // Get the window rectangle.
    RECT rc_w;
    GetWindowRect(hwnd, &rc_w);

    RECT rc_f = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&rc_f, WS_OVERLAPPEDWINDOW, FALSE, 0);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT row = 1;
    USHORT col = 1;
    bool resize_border = 0;

    int title_border = -rc_f.top;
    int sizing_border = -rc_f.left;

    // Determine if the point is at the top or bottom of the window.
    if (mouse.y >= rc_w.top && mouse.y < rc_w.top + title_border) {
        resize_border = (mouse.y < (rc_w.top + sizing_border));
        row = 0;
    }
    else if (mouse.y < rc_w.bottom && mouse.y >= rc_w.bottom - sizing_border) {
        row = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (mouse.x >= rc_w.left && mouse.x < rc_w.left + sizing_border) {
        col = 0; // left side
    }
    else if (mouse.x < rc_w.right && mouse.x >= rc_w.right - sizing_border) {
        col = 2; // right side
    }

    //    if (row == 0 && col != 2 && !resize_border && mouse.x > rc_w.right - 32) {
    //        // qDebug() << "Close";
    //        return HTCLOSE;
    //    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hit_tests[3][3] =
    {
        { HTTOPLEFT, resize_border ? HTTOP : HTCAPTION, HTTOPRIGHT },
        { HTLEFT, HTCLIENT, HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return hit_tests[row][col];
}

static bool
frameless_message(MSG *msg, HWND hwnd, LRESULT *lres)
{   
    if (msg->message == WM_NCHITTEST) {
        POINT mouse = { GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
        *lres = frameless_wm_nchittest(hwnd, mouse);
        return true;
    } else if (msg->message == WM_NCCALCSIZE) {
        *lres = 0;
        return true;
    }
    return false;
}
}

Q_DECLARE_METATYPE(QMargins)
#include <QApplication>
//#include <qpa/qplatformnativeinterface.h>

void Frameless::init(QWidget *widget)
{
    widget->setWindowFlags(Qt::FramelessWindowHint);

    widget->createWinId();

    RECT margins = {};
    AdjustWindowRectEx(&margins, WS_OVERLAPPEDWINDOW, FALSE, 0);

    DWMNCRENDERINGPOLICY val = DWMNCRP_ENABLED;
    HWND hwnd = (HWND)widget->windowHandle()->winId();
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &val, sizeof(DWMNCRENDERINGPOLICY));
    QtWin::extendFrameIntoClientArea(widget, -1, -1, -1, -1);


    // This will make syncing with DWM better (though I'm not sure what exactly is going on).
    // https://www.opengl.org/discussion_boards/showthread.php/164119-Making-composite-managers-go-along-with-OpenGL
    // Also look here (search for "fps.uiNumerator")
    // https://github.com/treblewinner/FB-Alpha/blob/master/src/burner/win32/dwmapi_core.cpp
    // More links:
    // - https://forums.geforce.com/default/topic/536046/geforce-drivers/windows-7-animation-lag/post/4001769/#4001769
    DWM_PRESENT_PARAMETERS dpp;
    memset(&dpp, 0, sizeof(DWM_PRESENT_PARAMETERS));
    dpp.cbSize = sizeof(dpp);
    dpp.fQueue = TRUE;
    dpp.cBuffer = 2;
    dpp.fUseSourceRate = FALSE;
    dpp.cRefreshesPerFrame = 1;
    dpp.eSampling = DWM_SOURCE_FRAME_SAMPLING_POINT;
    ::DwmSetPresentParameters(hwnd, &dpp);

//    QGuiApplication::platformNativeInterface()->
//        setWindowProperty(widget->windowHandle()->handle(),
//                          QStringLiteral("WindowsCustomMargins"),
//                          qVariantFromValue(QMargins(margins.left, margins.top, margins.right, margins.bottom)));


//    SetWindowLong(hwnd, GWL_STYLE,
//                  GetWindowLong(hwnd, GWL_STYLE) & ~ (WS_CAPTION | WS_THICKFRAME));
//    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
//                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
//                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

//    RECT window_rect;
//    GetWindowRect(hwnd, &window_rect);

//    // Notify frame change.
//    SetWindowPos(hwnd,
//        NULL,
//        window_rect.left, window_rect.top,
//        window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
//        SWP_FRAMECHANGED);
}

void Frameless::setWindowFlags(QWidget *widget)
{
    // widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
}

bool Frameless::event(QWidget *widget, QEvent *event)
{
    if (event->type() == QEvent::WinIdChange)
    {
        HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        // ??? ::SetProp(hwnd, L"private", (HANDLE)m_private);
        frameless_set(hwnd);
    }
    return false;
}

bool Frameless::nativeEvent(QWidget *widget, const QByteArray &eventType, void *mptr, long *rptr)
{
    Q_UNUSED(widget);
    Q_UNUSED(eventType);

    MSG *msg = (MSG*)mptr;
    // if (msg->hwnd == reinterpret_cast<HWND>(widget->winId())) {
    LRESULT &lres = *((LRESULT*)rptr);
    if (frameless_message(msg, msg->hwnd, &lres)) {
        return true;
    }
    // }
    return false;
}
#else
bool Frameless::event(QWidget *widget, QEvent *event)
{
   return false;
}

bool Frameless::nativeEvent(QWidget *widget, const QByteArray &eventType, void *mptr, long *rptr)
{
    return false;
}

void Frameless::setWindowFlags(QWidget *widget)
{}
#endif
