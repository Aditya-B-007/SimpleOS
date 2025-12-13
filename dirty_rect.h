#ifndef DIRTY_RECT_H
#define DIRTY_RECT_H

#include "graphics.h"

#define MAX_DIRTY_RECTS 32

typedef struct {
    int x, y, width, height;
} Rect;

/**
 * @brief Initializes or clears the dirty rectangle list.
 */
void dirty_rect_init();

/**
 * @brief Adds a new rectangle to the list of dirty regions.
 * This function will attempt to merge the new rectangle with existing ones.
 */
void dirty_rect_add(int x, int y, int width, int height);

/**
 * @brief Provides access to the list of dirty rectangles for the renderer.
 */
const Rect* dirty_rect_get_all(int* count);

#endif // DIRTY_RECT_H