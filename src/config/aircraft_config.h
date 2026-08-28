#ifndef AERODYN_AIRCRAFT_CONFIG_H
#define AERODYN_AIRCRAFT_CONFIG_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct AircraftRotorSpec {
    std::string id;
    std::array<double, 3> position_body_m{};
    std::array<double, 3> thrust_axis_body{};
    double spin_direction{0.0};
    double thrust_coefficient{0.0};
    double torque_coefficient{0.0};
    double maximum_speed_rad_s{0.0};
};

struct AircraftSpec {
    std::uint32_t schema_version{0};
    std::string aircraft_id;
    std::uint32_t revision{0};
    double mass_kg{0.0};
    std::array<double, 3> center_of_mass_m{};
    std::array<std::array<double, 3>, 3> inertia{};
    std::array<std::array<double, 3>, 3> inertia_inverse{};
    double gravity_m_s2{0.0};
    std::vector<AircraftRotorSpec> rotors;
};

struct PidAxisCoefficients {
    double kp{0.0};
    double ki{0.0};
    double kd{0.0};
    double integrator_limit{0.0};
    double output_limit{0.0};
};

struct ControllerCoefficientBundle {
    std::uint32_t schema_version{0};
    std::string bundle_id;
    std::string aircraft_id;
    std::uint32_t aircraft_revision{0};
    std::string status;
    double nominal_period_s{0.0};
    double minimum_period_s{0.0};
    double maximum_period_s{0.0};
    double rate_weight{0.0};
    std::array<PidAxisCoefficients, 3> attitude_pid{};
    bool physical_flight_approved{false};
};

bool loadAircraftSpec(const std::string& path, AircraftSpec& output,
                      std::string& error);
bool loadControllerCoefficientBundle(const std::string& path,
                                     const AircraftSpec& aircraft,
                                     ControllerCoefficientBundle& output,
                                     std::string& error);

#endif
