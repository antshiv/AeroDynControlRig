/**
 * @file sensor_simulator.h
 * @brief IMU sensor simulation module (gyroscope + accelerometer)
 */

#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include "core/module.h"

/**
 * @class SensorSimulatorModule
 * @brief Simulates IMU sensor measurements (gyro + accel) based on vehicle state
 *
 * This module generates synthetic sensor data by transforming true vehicle
 * state into body-frame measurements:
 *
 * **Gyroscope**: Reads angular velocity directly from state (with unit conversion)
 * **Accelerometer**: Transforms gravity vector into body frame using current attitude
 *
 * Future enhancements:
 * - Add sensor noise (white noise, bias drift)
 * - Include linear acceleration terms
 * - Model sensor saturation and quantization
 */
class SensorSimulatorModule : public Module {
public:
    /**
     * @brief Initialize sensor parameters
     * @param state Reference to simulation state
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Generate simulated sensor measurements
     *
     * Updates SimulationState::sensor with:
     * - gyro_rad_s: Angular velocity in body frame (rad/s)
     * - accel_mps2: Specific force in body frame (m/s²), including gravity
     *
     * @param dt Time step (seconds)
     * @param state Reference to simulation state (reads quaternion/angular_rate,
     *              writes sensor.gyro_rad_s and sensor.accel_mps2)
     */
    void update(double dt, SimulationState& state) override;

private:
    double gravity_{9.80665}; ///< Gravitational acceleration magnitude (m/s²)
};

#endif // SENSOR_SIMULATOR_H
