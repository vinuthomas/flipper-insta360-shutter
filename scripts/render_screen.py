#!/usr/bin/env python3
"""Render the shared control layout as an SVG preview with approximate fonts."""
import html
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="768" height="384" viewBox="0 0 128 64" role="img" aria-labelledby="title desc">',
         '<title id="title">Insta360 Shutter screen preview</title>',
         '<desc id="desc">Emulated ready state. Up changes mode, Left toggles screen, hold Right sleeps and OK triggers shutter. Fonts are approximate.</desc>',
         '<rect width="128" height="64" fill="#ef9b25"/>',
         '<g fill="#181300" stroke="#181300" stroke-width="1" shape-rendering="crispEdges">',
         '<rect width="128" height="13"/>']

def text(x, y, value, primary=False, color='#181300', anchor='start'):
    parts.append(f'<text x="{x}" y="{y}" fill="{color}" stroke="none" font-family="monospace" font-size="{9 if primary else 8}" font-weight="{700 if primary else 400}" text-anchor="{anchor}">{html.escape(value)}</text>')

text(3, 10, 'Insta360', True, '#ef9b25')
text(125, 10, 'Ready', color='#ef9b25', anchor='end')
primary = False
for line in (ROOT / 'remote_screen.h').read_text().splitlines():
    if 'canvas_set_font' in line:
        primary = 'FontPrimary' in line
    match = re.search(r'canvas_draw_str\(canvas, (\d+), (\d+), "([^"]+)"\)', line)
    if match:
        x, y, value = match.groups()
        text(x, y, value, primary)
    match = re.search(r'canvas_draw_frame\(canvas, (\d+), (\d+), (\d+), (\d+)\)', line)
    if match:
        x, y, w, h = map(int, match.groups())
        parts.append(f'<rect x="{x+.5}" y="{y+.5}" width="{w-1}" height="{h-1}" fill="none"/>')
parts.append('<path d="M0 52.5H128" fill="none"/>')
text(64, 63, 'Back: exit', anchor='middle')
parts.append('</g></svg>')
output = ROOT / 'docs/images/screen-preview.svg'
output.write_text('\n'.join(parts) + '\n')
print(output)
