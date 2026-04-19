# donut.c

## Simplified understanding

- torus (or donut) is represented by a circle rotating around another circle
- we're viewing the rotation from some fixed position in the world
- for every frame the torus under some rotation is constructed
    - for each piece of the torus fill the z-buffer
    - also calculate the luminance of the piece and save it after z-fighting
- uses ANSI escape sequences to clear the screen
- print the luminance and buffer (approximated by ASCII chars)
