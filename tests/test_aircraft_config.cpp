#include "config/aircraft_config.h"

#include <cmath>
#include <cstdio>

int main() {
    AircraftSpec aircraft;
    ControllerCoefficientBundle coefficients;
    std::string error;
    if (!loadAircraftSpec(ASR_DEFAULT_AIRCRAFT_SPEC, aircraft, error)) {
        std::fprintf(stderr, "aircraft load failed: %s\n", error.c_str());
        return 1;
    }
    if (aircraft.aircraft_id != "asr-reference-quad" ||
        aircraft.revision != 1u || aircraft.rotors.size() != 4u ||
        std::abs(aircraft.mass_kg - 1.2) > 1e-12 ||
        std::abs(aircraft.inertia_inverse[0][0] - 50.0) > 1e-12) {
        std::fprintf(stderr, "aircraft values do not match the accepted fixture\n");
        return 2;
    }
    if (!loadControllerCoefficientBundle(ASR_DEFAULT_COEFFICIENT_BUNDLE,
                                         aircraft, coefficients, error)) {
        std::fprintf(stderr, "coefficient load failed: %s\n", error.c_str());
        return 3;
    }
    if (coefficients.aircraft_id != aircraft.aircraft_id ||
        coefficients.aircraft_revision != aircraft.revision ||
        coefficients.physical_flight_approved ||
        coefficients.status != "simulation_only") {
        std::fprintf(stderr, "coefficient provenance gate failed\n");
        return 4;
    }

    AircraftSpec incompatible_aircraft = aircraft;
    incompatible_aircraft.revision += 1u;
    ControllerCoefficientBundle rejected_coefficients;
    if (loadControllerCoefficientBundle(ASR_DEFAULT_COEFFICIENT_BUNDLE,
                                        incompatible_aircraft,
                                        rejected_coefficients, error)) {
        std::fprintf(stderr, "coefficient revision mismatch was accepted\n");
        return 5;
    }
    std::puts("AeroDyn aircraft contract: all tests passed");
    return 0;
}
