#pragma once

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define _W(x) __W(x)
#define __W(x) L##x

#define VA_PRINTS(s)               \
    {                              \
        va_list argp;              \
        va_start(argp, fmt);       \
        vswprintf_s(s, fmt, argp); \
        va_end(argp);              \
    }

void DebugOut(const wchar_t *fmt, ...);
void DebugOutTitle(const wchar_t *fmt, ...);
void SetDebugWindow(HWND hwnd);