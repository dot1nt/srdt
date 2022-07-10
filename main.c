#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR "#ffffff"

Display *display;
Window window;
Window root;
int screen;

GC gc;

void Click(Display *display, int button) {
    XEvent event;
    memset (&event, 0, sizeof (event));
    event.xbutton.button = button;
    event.xbutton.same_screen = 1;
    event.type = ButtonPress;
    
    // Get to lowest window
    event.xbutton.subwindow = DefaultRootWindow(display);
    while (event.xbutton.subwindow) {
        event.xbutton.window = event.xbutton.subwindow;
        XQueryPointer(display, event.xbutton.window, &event.xbutton.root,
                        &event.xbutton.subwindow, &event.xbutton.x_root,
                        &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
                        &event.xbutton.state);
    }


    XSendEvent(display, PointerWindow, true, ButtonPressMask, &event);
    // no need to flush, XNextEvent does that
}

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

Window CreateWindow(void) {
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

    return window;
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

    bool pressed = false;
    bool start = false;
    bool create_window = true;

    int sx;
    int sy;

    XEvent event;
    Window window;

    while (1) {
        if (create_window) {
            window = CreateWindow();
            create_window = false;
        }
        XNextEvent(display, &event);

        switch (event.type) {
            case ButtonPress:
                // Left click
                if (event.xbutton.button == Button1) {
                    pressed = true;
                    start = true;
                    break;
                }

                // Not left click, pass on to background
                create_window = true;
                XDestroyWindow(display, window);
                Click(display, event.xbutton.button);
                break;

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
