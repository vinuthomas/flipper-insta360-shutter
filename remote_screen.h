#pragma once

// Fixed 128x64 control layout. Also used by scripts/render_screen.py.
static void remote_draw_controls(Canvas* canvas) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 3, 23, "Up: mode");
    canvas_draw_str(canvas, 3, 35, "Hold R: sleep");
    canvas_draw_str(canvas, 3, 49, "Left: screen on/off");
    canvas_draw_frame(canvas, 78, 16, 49, 24);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 94, 26, "OK");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 84, 36, "Shutter");
}
