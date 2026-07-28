# CTD 26 — KungFu Chess Server: Graphics

*KamaTech*

## Graphics: Image – Color & Transparency

1. **Paint.Net** – See the pixels. XY Axes in an image / screen.
2. **RGB** – Playing with color. Memory layout.
3. **Transparency** – The 4th channel. Jpeg vs PNG. Bit depth.
4. **Paint.Net** – Layers, overlay, flattening
5. **Paint.Net** – Put_text

## Graphics: Image – Color & Transparency (cont.)

**You are given:**

1. Basic Image reading and presentation library ([repo](#) + example):

   ```python
   class Img:
       def read(path)                       # with transparency
       def draw_on(other_img, x, y)         # with transparency
       def put_text(txt: str, x, y, font_size)  # write text on myself
       def show()                           # presents the image in game window
Animation images (a.k.a "Sprites") in a predefined folder structure
Graphics: Animation
Animations Explained

Serial, sometimes Circular
FPS, definitions Json.
State Machine (Mindset)
Graphics: Animation (cont.)
Game Controls

Mouse events
Relative window location and dynamic size
Screen pixels vs. image pixels
How to test yourself?
Additional Requirements
Moves log
Presenting score = "cost" of captured pieces.
Presenting player names
Observer
Mockup: a two-column moves-log panel (time + move, per player) flanking a
standard chess board with rank/file labels, a score readout above each log,
and the player's name below their column. See the original PDF for the
reference image.