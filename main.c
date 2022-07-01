#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define COLOR "#ffffff"

Display *display;
Window window;
Window root;
int screen;

GC gc;


long GetColor(char name) {
    XColor color;

    XParseColor(display, DefaultColormap(display, screen), COLOR, &color);
    XAllocColor(display, DefaultColormap(display, screen), &color);

    return color.pixel;
}

void Setup(void) {
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    root = RootWindow(display, 0);
}

void CreateWindow(void) {
    XSetWindowAttributes wa;

    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);

    wa.override_redirect = true;
    wa.background_pixmap = None;

    window = XCreateWindow(display, root, 0, 0, width, height,
                                0, DefaultDepth(display, screen),
                                CopyFromParent, DefaultVisual(display, screen),
                                CWOverrideRedirect, &wa );

    gc = XCreateGC(display, window, 0, 0);

    XLowerWindow(display, window);
    XSelectInput(display, window, ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    XMapWindow(display, window);

    XSetForeground(display, gc, GetColor(*COLOR));
    XSetWindowBackgroundPixmap(display, window, ParentRelative);
}

void DrawRectangle(int x, int y, int sx, int sy) {
    if (x > sx && y > sy) {
        XDrawRectangle(display, window, gc, sx, sy, 
                                            x - sx,
                                            y - sy);
    } else if (x < sx && y < sy) {
        XDrawRectangle(display, window, gc, x, y, 
                                            sx - x,
                                            sy - y);
    } else if (x < sx && y > sy) {
        XDrawRectangle(display, window, gc, x, sy, 
                                            sx - x, 
                                            abs(sy - y));
    } else if (x > sx && y < sy) {
        XDrawRectangle(display, window, gc, sx, y, 
                                            abs(x - sx), 
                                            sy - y);
    }
}

int main() {

    Setup();
    CreateWindow();

    bool pressed = false;
    bool start = false;

    int sx;
    int sy;

    XEvent event;

    while (1)
    {
        XNextEvent(display, &event);

        switch (event.type) {
            case ButtonPress:
                if (event.xbutton.button == Button1) {
                    pressed = true;
                    start = true;
                    break;
                }

            case ButtonRelease:
                XClearWindow(display, window);
                pressed = false;
                break;

            case MotionNotify:
                XClearWindow(display, window);

                int x = event.xmotion.x;
                int y = event.xmotion.y;

                if (start) {
                    start = false;
                    sx = x;
                    sy = y;
                }

                if (pressed) {
                    DrawRectangle(x, y, sx, sy);
                };
        }
    }

    XCloseDisplay(display);
 
    return 0;
 }
