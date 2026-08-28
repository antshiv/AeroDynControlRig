#include "config/aircraft_config.h"

#include <cmath>
#include <cstring>

#include <json-c/json.h>

namespace {

bool member(json_object* object, const char* name, json_type type,
            json_object** value, std::string& error) {
    if (!json_object_object_get_ex(object, name, value) ||
        !json_object_is_type(*value, type)) {
        error = std::string("missing or invalid field: ") + name;
        return false;
    }
    return true;
}

bool finiteNumber(json_object* object, const char* name, double& value,
                  std::string& error, bool allow_zero = false) {
    json_object* item = nullptr;
    if (!json_object_object_get_ex(object, name, &item) ||
        (!json_object_is_type(item, json_type_double) &&
         !json_object_is_type(item, json_type_int))) {
        error = std::string("missing or invalid number: ") + name;
        return false;
    }
    value = json_object_get_double(item);
    if (!std::isfinite(value) || (allow_zero ? value < 0.0 : value <= 0.0)) {
        error = std::string("out-of-range number: ") + name;
        return false;
    }
    return true;
}

bool vector3(json_object* object, const char* name,
             std::array<double, 3>& value, std::string& error) {
    json_object* array = nullptr;
    if (!member(object, name, json_type_array, &array, error) ||
        json_object_array_length(array) != 3u) {
        error = std::string("field must be a three-element array: ") + name;
        return false;
    }
    for (std::size_t index = 0; index < 3u; ++index) {
        json_object* item = json_object_array_get_idx(array, index);
        if (item == nullptr ||
            (!json_object_is_type(item, json_type_double) &&
             !json_object_is_type(item, json_type_int))) {
            error = std::string("non-numeric vector component: ") + name;
            return false;
        }
        value[index] = json_object_get_double(item);
        if (!std::isfinite(value[index])) {
            error = std::string("non-finite vector component: ") + name;
            return false;
        }
    }
    return true;
}

bool stringValue(json_object* object, const char* name, std::string& value,
                 std::string& error) {
    json_object* item = nullptr;
    if (!member(object, name, json_type_string, &item, error)) {
        return false;
    }
    value = json_object_get_string(item);
    if (value.empty()) {
        error = std::string("empty string: ") + name;
        return false;
    }
    return true;
}

bool unsignedValue(json_object* object, const char* name, std::uint32_t& value,
                   std::string& error) {
    json_object* item = nullptr;
    if (!member(object, name, json_type_int, &item, error)) {
        return false;
    }
    const std::int64_t parsed = json_object_get_int64(item);
    if (parsed <= 0 || parsed > UINT32_MAX) {
        error = std::string("out-of-range integer: ") + name;
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

bool invert3x3(const std::array<std::array<double, 3>, 3>& matrix,
               std::array<std::array<double, 3>, 3>& inverse) {
    const double a = matrix[0][0], b = matrix[0][1], c = matrix[0][2];
    const double d = matrix[1][0], e = matrix[1][1], f = matrix[1][2];
    const double g = matrix[2][0], h = matrix[2][1], i = matrix[2][2];
    const double determinant =
        a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    if (!std::isfinite(determinant) || std::abs(determinant) < 1e-15) {
        return false;
    }
    inverse = {{{(e * i - f * h) / determinant,
                 (c * h - b * i) / determinant,
                 (b * f - c * e) / determinant},
                {(f * g - d * i) / determinant,
                 (a * i - c * g) / determinant,
                 (c * d - a * f) / determinant},
                {(d * h - e * g) / determinant,
                 (b * g - a * h) / determinant,
                 (a * e - b * d) / determinant}}};
    return true;
}

bool loadPid(json_object* root, const char* name, PidAxisCoefficients& output,
             std::string& error) {
    json_object* axis = nullptr;
    if (!member(root, name, json_type_object, &axis, error)) {
        return false;
    }
    return finiteNumber(axis, "kp", output.kp, error, true) &&
           finiteNumber(axis, "ki", output.ki, error, true) &&
           finiteNumber(axis, "kd", output.kd, error, true) &&
           finiteNumber(axis, "integrator_limit", output.integrator_limit,
                        error, true) &&
           finiteNumber(axis, "output_limit", output.output_limit, error);
}

}  // namespace

bool loadAircraftSpec(const std::string& path, AircraftSpec& output,
                      std::string& error) {
    output = {};
    json_object* root = json_object_from_file(path.c_str());
    if (root == nullptr) {
        error = "unable to parse aircraft specification: " + path;
        return false;
    }

    AircraftSpec parsed;
    json_object* frames = nullptr;
    json_object* mass_properties = nullptr;
    json_object* environment = nullptr;
    json_object* rotors = nullptr;
    std::string inertial;
    std::string body;
    std::string quaternion;
    bool valid = unsignedValue(root, "schema_version", parsed.schema_version,
                               error) &&
                 parsed.schema_version == 1u &&
                 stringValue(root, "aircraft_id", parsed.aircraft_id, error) &&
                 unsignedValue(root, "revision", parsed.revision, error) &&
                 member(root, "frames", json_type_object, &frames, error) &&
                 stringValue(frames, "inertial", inertial, error) &&
                 stringValue(frames, "body", body, error) &&
                 stringValue(frames, "quaternion", quaternion, error) &&
                 inertial == "NED" && body == "FRD" &&
                 quaternion == "body_to_inertial_scalar_first" &&
                 member(root, "mass_properties", json_type_object,
                        &mass_properties, error) &&
                 finiteNumber(mass_properties, "mass_kg", parsed.mass_kg,
                              error) &&
                 vector3(mass_properties, "center_of_mass_m",
                         parsed.center_of_mass_m, error) &&
                 member(root, "environment", json_type_object, &environment,
                        error) &&
                 finiteNumber(environment, "gravity_m_s2",
                              parsed.gravity_m_s2, error) &&
                 member(root, "rotors", json_type_array, &rotors, error);
    if (valid && (json_object_array_length(rotors) == 0u ||
                  json_object_array_length(rotors) > 8u)) {
        error = "rotor count must be between one and eight";
        valid = false;
    }

    json_object* inertia = nullptr;
    if (valid && (!member(mass_properties, "inertia_kg_m2", json_type_array,
                          &inertia, error) ||
                  json_object_array_length(inertia) != 3u)) {
        error = "inertia_kg_m2 must be a 3x3 matrix";
        valid = false;
    }
    for (std::size_t row = 0; valid && row < 3u; ++row) {
        json_object* row_value = json_object_array_get_idx(inertia, row);
        if (row_value == nullptr ||
            !json_object_is_type(row_value, json_type_array) ||
            json_object_array_length(row_value) != 3u) {
            error = "inertia_kg_m2 must be a 3x3 matrix";
            valid = false;
            break;
        }
        for (std::size_t column = 0; column < 3u; ++column) {
            json_object* item = json_object_array_get_idx(row_value, column);
            if (item == nullptr ||
                (!json_object_is_type(item, json_type_double) &&
                 !json_object_is_type(item, json_type_int))) {
                error = "inertia_kg_m2 contains a non-numeric value";
                valid = false;
                break;
            }
            parsed.inertia[row][column] = json_object_get_double(item);
        }
    }
    for (std::size_t row = 0; valid && row < 3u; ++row) {
        if (parsed.inertia[row][row] <= 0.0) {
            error = "inertia diagonal must be positive";
            valid = false;
        }
        for (std::size_t column = 0; column < 3u; ++column) {
            if (!std::isfinite(parsed.inertia[row][column]) ||
                std::abs(parsed.inertia[row][column] -
                         parsed.inertia[column][row]) > 1e-10) {
                error = "inertia matrix must be finite and symmetric";
                valid = false;
            }
        }
    }
    if (valid && !invert3x3(parsed.inertia, parsed.inertia_inverse)) {
        error = "inertia matrix is singular";
        valid = false;
    }

    for (std::size_t index = 0;
         valid && index < json_object_array_length(rotors); ++index) {
        json_object* rotor = json_object_array_get_idx(rotors, index);
        AircraftRotorSpec rotor_spec;
        std::string direction;
        valid = rotor != nullptr &&
                json_object_is_type(rotor, json_type_object) &&
                stringValue(rotor, "id", rotor_spec.id, error) &&
                vector3(rotor, "position_body_m",
                        rotor_spec.position_body_m, error) &&
                vector3(rotor, "thrust_axis_body",
                        rotor_spec.thrust_axis_body, error) &&
                stringValue(rotor, "spin_direction", direction, error) &&
                finiteNumber(rotor, "thrust_coefficient_n_per_rad_s2",
                             rotor_spec.thrust_coefficient, error) &&
                finiteNumber(rotor, "torque_coefficient_nm_per_rad_s2",
                             rotor_spec.torque_coefficient, error, true) &&
                finiteNumber(rotor, "maximum_speed_rad_s",
                             rotor_spec.maximum_speed_rad_s, error);
        const double axis_norm = std::sqrt(
            rotor_spec.thrust_axis_body[0] * rotor_spec.thrust_axis_body[0] +
            rotor_spec.thrust_axis_body[1] * rotor_spec.thrust_axis_body[1] +
            rotor_spec.thrust_axis_body[2] * rotor_spec.thrust_axis_body[2]);
        if (valid && std::abs(axis_norm - 1.0) > 1e-6) {
            error = "rotor thrust axis must be unit length";
            valid = false;
        }
        if (valid && direction != "CW" && direction != "CCW") {
            error = "spin_direction must be CW or CCW";
            valid = false;
        }
        rotor_spec.spin_direction = direction == "CW" ? 1.0 : -1.0;
        if (valid) {
            parsed.rotors.push_back(rotor_spec);
        }
    }

    json_object_put(root);
    if (!valid) {
        if (error.empty()) {
            error = "aircraft specification violates the v1 contract";
        }
        return false;
    }
    output = std::move(parsed);
    error.clear();
    return true;
}

bool loadControllerCoefficientBundle(const std::string& path,
                                     const AircraftSpec& aircraft,
                                     ControllerCoefficientBundle& output,
                                     std::string& error) {
    output = {};
    json_object* root = json_object_from_file(path.c_str());
    if (root == nullptr) {
        error = "unable to parse coefficient bundle: " + path;
        return false;
    }
    ControllerCoefficientBundle parsed;
    json_object* timing = nullptr;
    json_object* pid = nullptr;
    json_object* acceptance = nullptr;
    json_object* approved = nullptr;
    bool valid = unsignedValue(root, "schema_version", parsed.schema_version,
                               error) && parsed.schema_version == 1u &&
                 stringValue(root, "bundle_id", parsed.bundle_id, error) &&
                 stringValue(root, "aircraft_id", parsed.aircraft_id, error) &&
                 unsignedValue(root, "aircraft_revision",
                               parsed.aircraft_revision, error) &&
                 stringValue(root, "status", parsed.status, error) &&
                 parsed.aircraft_id == aircraft.aircraft_id &&
                 parsed.aircraft_revision == aircraft.revision &&
                 member(root, "timing", json_type_object, &timing, error) &&
                 finiteNumber(timing, "nominal_period_s",
                              parsed.nominal_period_s, error) &&
                 finiteNumber(timing, "minimum_period_s",
                              parsed.minimum_period_s, error) &&
                 finiteNumber(timing, "maximum_period_s",
                              parsed.maximum_period_s, error) &&
                 parsed.minimum_period_s <= parsed.nominal_period_s &&
                 parsed.nominal_period_s <= parsed.maximum_period_s &&
                 member(root, "attitude_pid", json_type_object, &pid, error) &&
                 finiteNumber(pid, "rate_weight", parsed.rate_weight, error,
                              true) &&
                 loadPid(pid, "roll", parsed.attitude_pid[0], error) &&
                 loadPid(pid, "pitch", parsed.attitude_pid[1], error) &&
                 loadPid(pid, "yaw", parsed.attitude_pid[2], error) &&
                 member(root, "acceptance", json_type_object, &acceptance,
                        error) &&
                 member(acceptance, "physical_flight_approved",
                        json_type_boolean, &approved, error);
    if (valid) {
        parsed.physical_flight_approved = json_object_get_boolean(approved);
        if (parsed.status != "simulation_only" &&
            parsed.status != "hil_validated" &&
            parsed.status != "flight_tested") {
            error = "unsupported coefficient acceptance status";
            valid = false;
        } else if (parsed.status != "flight_tested" &&
                   parsed.physical_flight_approved) {
            error = "only flight-tested coefficients can be flight approved";
            valid = false;
        }
    } else if (error.empty()) {
        error = "coefficient bundle does not match the aircraft specification";
    }
    json_object_put(root);
    if (!valid) {
        return false;
    }
    output = std::move(parsed);
    error.clear();
    return true;
}
