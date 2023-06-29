#include <stdio.h>
#include <math.h>

#include "chroma.h"

#define H_SYNC_RISE -0.1
#define H_SYNC_FALL -0.5

int main() {
    FILE* fp = 0;
    fopen_s(&fp, "C:\\Users\\ris2006\\Desktop\\test.csv", "w");
    if (nullptr == fp) {
        return 0;
    }

    Chroma* c = new Chroma();
    c->sync();

    int data_count = 0;
    for (int i = 0; i < 512; i++) {
        if (c->is_sync()) {
            fprintf(fp, "%.3f\n", c->step());
            continue;
        }

        if (data_count < 256) {
            int f = 32 * data_count / 256;
            double th = 8 * atan(1) * f / 32.0;
            c->m_re = cos(th);
            c->m_im = sin(th);
            data_count++;
        } else {
            c->sync();
            data_count = 0;
        }

        fprintf(fp, "%.3f\n", c->step());
    }

    fclose(fp);
    return 0;
}