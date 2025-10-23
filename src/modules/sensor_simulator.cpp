#include "modules/sensor_simulator.h"

#include <cmath>

#include "attitude/quaternion.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include "core/simulation_state.h"

void SensorSimulatorModule::initialize(SimulationState& state) {
    state.sensor.gyro_rad_s = glm::vec3(0.0f);
    state.sensor.accel_mps2 = glm::vec3(0.0f, 0.0f, static_cast<float>(-gravity_));
}

void SensorSimulatorModule::update(double dt, SimulationState& state) {
    (void)dt;

    double yaw_rate_rad_s = deg2rad(state.rotation_speed_deg_per_sec);
    state.sensor.gyro_rad_s = glm::vec3(0.0f, 0.0f, static_cast<float>(yaw_rate_rad_s));

    double dcm[3][3];
    euler_to_dcm(&state.euler, dcm);

    double gravity_world[3] = {0.0, 0.0, -gravity_};
    double gravity_body[3] = {
        dcm[0][0] * gravity_world[0] + dcm[1][0] * gravity_world[1] + dcm[2][0] * gravity_world[2],
        dcm[0][1] * gravity_world[0] + dcm[1][1] * gravity_world[1] + dcm[2][1] * gravity_world[2],
        dcm[0][2] * gravity_world[0] + dcm[1][2] * gravity_world[1] + dcm[2][2] * gravity_world[2]
    };

    state.sensor.accel_mps2 = glm::vec3(static_cast<float>(gravity_body[0]),
                                        static_cast<float>(gravity_body[1]),
                                        static_cast<float>(gravity_body[2]));
}
