/**
 * @file module.h
 * @brief Base interface for simulation modules in the AeroDynControlRig
 */

#ifndef MODULE_H
#define MODULE_H

class SimulationState;

/**
 * @class Module
 * @brief Abstract base class for all simulation modules
 *
 * Modules encapsulate discrete simulation components (dynamics, sensors, estimators, etc.)
 * and operate on the shared SimulationState. Each module is initialized once and updated
 * each simulation tick.
 *
 * @see SimulationState
 */
class Module {
public:
    virtual ~Module() = default;

    /**
     * @brief Initialize the module with the current simulation state
     *
     * Called once during application startup. Override to set initial values
     * in the simulation state or prepare internal resources.
     *
     * @param state Reference to the shared simulation state
     */
    virtual void initialize(SimulationState& state) {}

    /**
     * @brief Update the module for one simulation timestep
     *
     * Called every frame (or simulation tick) to advance the module's state.
     * Implement module-specific physics, algorithms, or state transitions here.
     *
     * @param dt Time elapsed since last update (seconds)
     * @param state Reference to the shared simulation state (read/write)
     */
    virtual void update(double dt, SimulationState& state) = 0;
};

#endif // MODULE_H
