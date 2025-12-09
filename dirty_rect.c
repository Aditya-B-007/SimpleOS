#include "dirty_rect.h"
#include <stdbool.h>

static Rect dirty_rects[MAX_DIRTY_RECTS];
static int dirty_rect_count = 0;

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }

void dirty_rect_init() {
    dirty_rect_count = 0;
}

void dirty_rect_add(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    Rect new_rect = {x, y, width, height};
restart_scan:
    for (int i = 0; i < dirty_rect_count; ++i) {
        Rect* existing = &dirty_rects[i];
        if (!(new_rect.x >= existing->x + existing->width ||
              new_rect.x + new_rect.width <= existing->x ||
              new_rect.y >= existing->y + existing->height ||
              new_rect.y + new_rect.height <= existing->y))
        {
            new_rect.x = min(existing->x, new_rect.x);
            new_rect.y = min(existing->y, new_rect.y);
            int x2 = max(existing->x + existing->width, new_rect.x + new_rect.width);
            int y2 = max(existing->y + existing->height, new_rect.y + new_rect.height);
            new_rect.width = x2 - new_rect.x;
            new_rect.height = y2 - new_rect.y;
            dirty_rects[i] = dirty_rects[dirty_rect_count - 1];
            dirty_rect_count--;
            goto restart_scan;
        }
    }
    if (dirty_rect_count < MAX_DIRTY_RECTS) {
        dirty_rects[dirty_rect_count++] = new_rect;
    } else {
        for (int i = 1; i < dirty_rect_count; i++) {
             dirty_rect_add(dirty_rects[i].x, dirty_rects[i].y, dirty_rects[i].width, dirty_rects[i].height);
        }
        dirty_rect_count = 1;
    }
}

const Rect* dirty_rect_get_all(int* count) {
    *count = dirty_rect_count;
    return dirty_rects;
}