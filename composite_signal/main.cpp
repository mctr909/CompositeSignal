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

    int span = 0;
    for (int i = 0; i < 1000; i++) {
        if (c->is_sync_end()) {
            span++;
        } else {
            fprintf(fp, "%.3f\n", c->step());
            continue;
        }

        if (span < 300) {
            int f = 24 * span / 300;
            double th = 8 * atan(1) * f / 24.0;
            c->m_re = cos(th) * (0.5 + 4 * f / 24) / 4.5;
            c->m_im = sin(th) * (0.5 + 4 * f / 24) / 4.5;
        } else {
            c->sync();
            span = 0;
        }

        fprintf(fp, "%.3f\n", c->step());
    }

    fclose(fp);
    return 0;
}