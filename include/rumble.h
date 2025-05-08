#ifndef RUMBLE_H
#define RUMBLE_H

#include <cstdint>
#include <math.h>

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
    static HDRumbleDataInternal from_side_config(const HDRumbleSideConfig &p_config) {
        double high_band_freq_float = p_config.high.frequency;
        double high_band_amp_float = p_config.high.amplitude; // TODO: clamp 0-1
        
        double low_band_freq_float = p_config.low.frequency;
        double low_band_amp_float = p_config.low.amplitude; // TODO: clamp 0-1

        uint16_t hf = frequency_to_hf_range(high_band_freq_float);
        uint16_t hf_amp = amplitude_to_hf_amp(high_band_amp_float);

        uint8_t byte0 = (uint8_t)(hf & 0xFF);
        uint8_t byte1 = (uint8_t)(hf_amp + ((hf >> 8) & 0xFF));

        uint16_t lf = frequency_to_lf_range(low_band_freq_float);
        uint16_t lf_amp = amplitude_to_lf_amp(low_band_amp_float);

        uint8_t byte2 = (uint8_t)(lf + ((lf_amp >> 8) & 0xFF));
        uint8_t byte3 = (uint8_t)(lf_amp & 0xFF);

        HDRumbleDataInternal new_data;
        new_data.high_band_lower_freq = byte0;
        new_data.high_band_amp = byte1;
        new_data.low_band_freq = byte2;
        new_data.low_band_amp = byte3;
        return new_data;
    }

private:
    static uint8_t frequency_to_hex(double p_freq) {
        if (p_freq < 0.0) p_freq = 0.0;
        else if (p_freq > 1252.0) p_freq = 1252.0;
        return (uint8_t)round(log2(p_freq / 10.0) * 32.0);
    }

    static uint8_t amplitude_to_hex(double p_amp) {
        if (p_amp > 1.0) p_amp = 1.0;
        if (p_amp > 0.23) return (uint8_t)round(log2(p_amp * 8.7) * 32.0);
        else if (p_amp > 0.12) return (uint8_t)round(log2(p_amp * 17.0) * 16.0);
        else return 0;
    }

    static uint16_t frequency_to_hf_range(double p_freq) {
        return (uint16_t)((frequency_to_hex(p_freq) - 0x60) * 4);
    }

    static uint8_t frequency_to_lf_range(double p_freq) {
        return (uint8_t)(frequency_to_hex(p_freq) - 0x40);
    }

    static uint16_t amplitude_to_hf_amp(double p_amp) {
        return (uint16_t)(amplitude_to_hex(p_amp) * 2);
    }

    static uint8_t amplitude_to_lf_amp(double p_amp) {
        return (uint8_t)(amplitude_to_hex(p_amp) / 2 + 64);
    }
};

#endif // RUMBLE_H
