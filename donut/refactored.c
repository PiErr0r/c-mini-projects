#include <stdio.h>
#include <string.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 22
#define LEN (WIDTH*HEIGHT) // 80 * 22
#define CENTER_X 40
#define CENTER_Y 12
#define PI2 6.28

#define DRX 0.04
#define DRZ 0.02

int main(void)
{
    float rot_x = 0, rot_z = 0;
    float z_buffer[LEN];
    char buffer[LEN];
    char space = ' ';
    printf("\x1b[2J"); // clear screen
    for(;;) {
        float x_sin = sin(rot_x);
        float x_cos = cos(rot_x);
        float z_sin = sin(rot_z);
        float z_cos = cos(rot_z);
        memset(buffer, (int)space, LEN * sizeof(char)); // fill b with spaces
        memset(z_buffer, 0, LEN * sizeof(float)); // fill z buffer with 0
        for(float theta = 0; theta < PI2; theta += 0.07) {
            float theta_cos = cos(theta);
            float theta_sin = sin(theta);
            for(float phi = 0; phi < PI2; phi += 0.02) {
                float phi_sin = sin(phi);
                float phi_cos = cos(phi);
                float h = theta_cos+2;
                // calculate Z coord
                float z_inv = 1/(phi_sin*h*x_sin + theta_sin*x_cos + 5);
                float t = phi_sin*h*x_cos - theta_sin*x_sin;
                int x = CENTER_X + 30 * z_inv * (phi_cos*h*z_cos - t*z_sin);
                int y = CENTER_Y + 15 * z_inv * (phi_cos*h*z_sin + t*z_cos);
                int coord = x + 80 * y;
                // luminance is in interval [-sqrt(2), sqrt(2)]
                float luminance = (
                        (theta_sin*x_sin - phi_sin*theta_cos*x_cos)*z_cos -
                        phi_sin*theta_cos*x_sin -
                        theta_sin*x_cos -
                        phi_cos*theta_cos*z_sin);
                // normalize luminance to [0, 11]
                // sqrt(2) * 8 = 11.3
                int N = 8 * luminance;
                // check we're in bounds and z coord is closer than previous
                if (0 < y && y < 22 && 0 < x && x < 80 && z_inv > z_buffer[coord]) {
                    z_buffer[coord] = z_inv;
                    // disregard negative luminance index (it means surface is looking away)
                    buffer[coord] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
                }
            }
        }
        printf("\x1b[H"); // move cursor to position (1, 1)
        for (int k = 0; k < LEN + 1; ++k)
            putchar(k % 80 != 0 ? buffer[k] : '\n'); // every 80-th character print new line
        long S = 0;
        for (volatile int ii = 0; ii < 10000000; ++ii) {
            // NOP - slow down
            S += ii;
        }
        rot_x += DRX;
        rot_z += DRZ;
    }
    return 0;
}
