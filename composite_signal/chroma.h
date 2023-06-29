#define CHROMA_BASE_LEVEL  0.3
#define CHROMA_SYNC_CYCLES 10

class Chroma {
private:
    const int    CYCLE_PER_SAMPLES = 4;
    const int    BURST_PAD = 30;
    const int    BURST_END = BURST_PAD + CYCLE_PER_SAMPLES * CHROMA_SYNC_CYCLES;
    const int    SYNC_END = BURST_END + BURST_PAD;
    const double SYNC_LIMIT = 0.050625; // (CHROMA_BASE_LEVEL*0.75)^2
    const double PI2 = 6.283185307;
    const double INV_FACT2 = 5.00000000e-01;
    const double INV_FACT3 = 1.66666667e-01;
    const double INV_FACT4 = 4.16666667e-02;
    const double INV_FACT5 = 8.33333333e-03;
    const double INV_FACT6 = 1.38888889e-03;
    const double INV_FACT7 = 1.98412698e-04;
    const double INV_FACT8 = 2.48015873E-05;
    const double INV_FACT9 = 2.75573192E-06;

public:
    double m_re = 0.0;
    double m_im = 0.0;
    double m_base_rms = 0.0;

private:
    int m_sync_samples = 0;
    int m_sync_cycles = 0;
    double m_signal_delay = 0.0;
    double m_diff_delay = 0.0;
    double m_delta_re = 0.0;
    double m_delta_im = 0.0;
    double m_osc_re = 0.0;
    double m_osc_im = 0.0;
    double m_rms = 0.0;

public:
    void sync() {
        m_sync_samples = 0;
        m_sync_cycles = 0;
    }

    inline bool is_sync_end() {
        return SYNC_END <= m_sync_samples;
    }

    inline double step() {
        if (m_sync_samples < BURST_PAD) {
            m_sync_samples++;
            return 0.0;
        }
        if (BURST_PAD == m_sync_samples) {
            const double rad = PI2 / CYCLE_PER_SAMPLES;
            const double rad_2 = rad * rad;
            m_delta_re = INV_FACT8;
            m_delta_re *= rad_2;
            m_delta_re -= INV_FACT6;
            m_delta_re *= rad_2;
            m_delta_re += INV_FACT4;
            m_delta_re *= rad_2;
            m_delta_re -= INV_FACT2;
            m_delta_re *= rad_2;
            m_delta_re += 1.0;
            m_delta_im = INV_FACT9;
            m_delta_im *= rad_2;
            m_delta_im -= INV_FACT7;
            m_delta_im *= rad_2;
            m_delta_im += INV_FACT5;
            m_delta_im *= rad_2;
            m_delta_im -= INV_FACT3;
            m_delta_im *= rad_2;
            m_delta_im += 1.0;
            m_delta_im *= rad;
            m_osc_re = CHROMA_BASE_LEVEL;
            m_osc_im = 0.0;
        }
        double current_re = m_osc_re;
        double current_im = m_osc_im;
        m_osc_re = current_re * m_delta_re - current_im * m_delta_im;
        m_osc_im = current_re * m_delta_im + current_im * m_delta_re;
        if (m_sync_samples < BURST_END) {
            m_sync_samples++;
            return current_im;
        }
        if (m_sync_samples < SYNC_END) {
            m_sync_samples++;
            return 0.0;
        }
        return (m_re * current_im + m_im * current_re);
    }

    inline void step(double signal) {
        double diff = signal - m_signal_delay;
        m_signal_delay = signal;

        m_rms = m_rms * 0.9 + signal * 0.1;

        if (is_sync_end()) {
            double current_re = m_osc_re;
            double current_im = m_osc_im;
            m_osc_re = current_re * m_delta_re - current_im * m_delta_im;
            m_osc_im = current_re * m_delta_im + current_im * m_delta_re;
            m_re = m_diff_delay * m_osc_im - diff * current_im;
            m_im = m_diff_delay * m_osc_re - diff * current_re;
            m_diff_delay = diff;
            return;
        }

        if (SYNC_LIMIT <= m_rms && diff < 0.0 && 0.0 <= m_diff_delay) {
            if (0 == m_sync_cycles) {
                m_sync_samples = 0;
            }
            m_sync_cycles++;
        }
        m_sync_samples++;
        m_diff_delay = diff;

        if (is_sync_end()) {
            double rad = PI2 * (m_sync_cycles - 1.0) / m_sync_samples;
            double rad_2 = rad * rad;
            m_delta_re = INV_FACT8;
            m_delta_re *= rad_2;
            m_delta_re -= INV_FACT6;
            m_delta_re *= rad_2;
            m_delta_re += INV_FACT4;
            m_delta_re *= rad_2;
            m_delta_re -= INV_FACT2;
            m_delta_re *= rad_2;
            m_delta_re += 1.0;
            m_delta_im = INV_FACT9;
            m_delta_im *= rad_2;
            m_delta_im -= INV_FACT7;
            m_delta_im *= rad_2;
            m_delta_im += INV_FACT5;
            m_delta_im *= rad_2;
            m_delta_im -= INV_FACT3;
            m_delta_im *= rad_2;
            m_delta_im += 1.0;
            m_delta_im *= rad;
            m_osc_re = 0.707 * m_delta_re - 0.707 * m_delta_im;
            m_osc_im = 0.707 * m_delta_im + 0.707 * m_delta_re;
            m_base_rms = m_rms;
        }
    }
};