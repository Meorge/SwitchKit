#ifndef RUMBLE_H
#define RUMBLE_H

struct HDRumbleFreqAmpConfig {
    double frequency = 0.0;
    double amplitude = 0.0;

    HDRumbleFreqAmpConfig(double p_freq, double p_amp): frequency(p_freq), amplitude(p_amp) {}
};

struct HDRumbleSideConfig {
    HDRumbleFreqAmpConfig high;
    HDRumbleFreqAmpConfig low;

    HDRumbleSideConfig() : high(320, 0), low(160, 0) {}
};

struct HDRumbleConfig {
    HDRumbleSideConfig left;
    HDRumbleSideConfig right;

    HDRumbleConfig(): left(), right() {}
};

// Internal
struct HDRumbleDataInternal {
    uint8_t high_band_lower_freq;
    uint8_t high_band_amp;
    uint8_t low_band_freq;
    uint8_t low_band_amp;

public:
    static HDRumbleDataInternal from_side_config(const HDRumbleSideConfig &p_config);

private:
    static uint8_t frequency_to_hex(double p_freq);
    static uint8_t amplitude_to_hex(double p_amp);
    static uint16_t frequency_to_hf_range(double p_freq);
    static uint8_t frequency_to_lf_range(double p_freq);
    static uint16_t amplitude_to_hf_amp(double p_amp);
    static uint8_t amplitude_to_lf_amp(double p_amp);
};

#endif // RUMBLE_H
