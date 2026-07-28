// HX711S strain gauge sensor support for bed probing
//
// Copyright (C) 2026 Timo V
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#ifndef __SENSOR_HX711S_H__
#define __SENSOR_HX711S_H__

#include <stdint.h>
#include <string.h>
#include "autoconf.h"   // CONFIG_CLOCK_FREQ
#include "board/gpio.h"
#include "sched.h"

// Forward declaration for trsync
struct trsync;

/****************************************************************
 * Configuration Constants
 ****************************************************************/

// Maximum number of sensors (4 physical + 1 fusion channel)
#define HX711S_MAX_SENSOR_NUM   5
// Maximum sliding window data points
#define HX711S_MAX_DATA_NUM     16
// Maximum calibration samples per sensor. This sizes a static buffer of
// HX711S_MAX_SENSORS * HX711S_MAX_CAL_SAMPLES * 4 bytes, so it is the single
// largest term in the driver's memory footprint. The probe path asks for 30
// (see calibration_start in klippy/extras/hx711s.py); the ceiling only bounds
// the manual HX711S_CALIBRATE gcode. Keep it in step with that command's
// maxval, and note that calibration_collect sorts in place at O(n^2).
#define HX711S_MAX_CAL_SAMPLES  128
// Maximum sensors per device
#define HX711S_MAX_SENSORS      4

// The high-pass cutoff and pi live in klippy/extras/hx711s.py now, because
// the host computes the filter coefficient and sends it.

// Status report cadence, two seconds in timer ticks. Compile-time constant, so
// the deadline arithmetic needs no divide. Stays under 2^31 for every supported
// clock, which is what timer_is_before() requires to compare correctly.
#define HX711S_HEARTBEAT_TICKS ((uint32_t)(CONFIG_CLOCK_FREQ * 2u))

/****************************************************************
 * Enumerations
 ****************************************************************/

// Sensor operating flags
enum hx711s_flags {
    HX711S_FLAG_START = 1 << 0,
    HX711S_FLAG_AWAIT_HOMING = 1 << 1,
    HX711S_FLAG_PENDING = 1 << 2,  // ADC data ready, task should read
};

// Strain gauge mode (chip and bridge type)
enum hx711s_sg_mode {
    SG_MODE_HX711_FULL_BRIDGE = 0,
    SG_MODE_HX711_HALF_BRIDGE = 1,
    SG_MODE_HX717_FULL_BRIDGE = 2,
    SG_MODE_HX717_HALF_BRIDGE = 3,
};

// Probe check command modes
enum hx711s_probe_mode {
    PROBE_CHECK_MODE_NONE        = 0,
    PROBE_CHECK_MODE_CALIBRATION = 3,
};

/****************************************************************
 * Data Structures
 ****************************************************************/

// High-pass filter parameters
struct hx711s_hpf_params {
    int32_t vi;             // Current input
    int32_t vi_prev;        // Previous input
    int32_t vo;             // Current output
    int32_t vo_prev;        // Previous output
    // Precomputed filter coefficient, sent by the host. Deriving it here
    // took three float divides per sample, which links __divsf3 and gets the
    // image rejected by scripts/check-software-div.sh on any chip without a
    // divide instruction. See _hpf_coeff in klippy/extras/hx711s.py, which
    // reproduces the float32 sequence this used to run, bit for bit.
    float coeff;
};

// Main HX711S sensor structure
struct hx711s_sensor {
    struct timer timer;
    uint32_t oid;
    uint32_t rest_ticks;
    uint32_t sample_period;         // Sampling period in microseconds
    uint32_t enable_channels;       // Enabled channel bitmask
    uint32_t enable_hpf;            // Enable high-pass filter
    uint32_t enable_shake_filter;   // Enable shake/vibration filter
    // find_index_mode bits:
    //   bit 0: apply slope compensation
    //   bits 1-2: rollback method (00=linear, 01=backward threshold, 10=forward)
    //   bit 3: slope calculation method
    uint32_t find_index_mode;
    uint32_t next_heartbeat_tick;   // Clock at which the next status report is due

    // Probe state
    int32_t probe_check_cmd;
    uint8_t flags;                  // bit 0 = START, bit 1 = AWAIT_HOMING
    uint8_t pending_sensor;          // sensor index found ready by timer ISR
    uint32_t homing_clock;          // Clock at which homing starts and triggers are awaited
    uint8_t is_calibration;         // bit 0 = calibration mode, bit 7 = calibration complete, bi 0 = ch1, 1 = ch2, etc.
    uint8_t is_trigger;             // Last trigger (bit coded by channel) bit 0 = ch1, 1 = ch2, etc.; bit 5 = fusion
    uint8_t trigger_index;          // Index of the triggered point on the bed
    uint32_t trigger_tick;          // Tick count when trigger was first detected

    // trsync for homing/probing integration
    struct trsync *ts;
    uint8_t trigger_reason;
    uint8_t error_reason;
    uint8_t is_homing;

    // Sensor configuration
    uint32_t hx711_count;
    uint32_t sg_mode;           // Chip/bridge mode
    uint8_t install_dir;        // Installation direction (0=negative, 1=positive)
    int32_t times_read;

    // GPIO pins (for HX711 bit-bang mode)
    struct gpio_out clks[HX711S_MAX_SENSORS];
    struct gpio_in sdos[HX711S_MAX_SENSORS];

    // Timestamps (index 4 = fusion timestamp)
    uint32_t time_stamp[5];

    // Sensor values
    int32_t init_values[HX711S_MAX_SENSORS];         // Baseline values
    int32_t amplitude_values[HX711S_MAX_SENSORS];    // Shake amplitude
    int32_t sample_values[HX711S_MAX_SENSORS];       // Raw ADC values
    int32_t max_data_num;                            // Sliding window size
    int32_t calibration_values[HX711S_MAX_SENSORS];  // Calibrated values
    int32_t fusion_filter_value;                     // Fused sensor value

    // Threshold and filter parameters
    int32_t kalman_q[HX711S_MAX_SENSORS];   // Used as ADC threshold
    int32_t kalman_r[HX711S_MAX_SENSORS];   // Used as shake amplitude threshold
    int32_t max_th;                          // Maximum trigger threshold
    int32_t min_th;                          // Minimum trigger threshold
};

#endif // __SENSOR_HX711S_H__
