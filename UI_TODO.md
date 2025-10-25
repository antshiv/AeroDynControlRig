# UI Overhaul TODO

Planning for the state estimation dashboard refresh to match the mock (`vision/state_estimation_view/stitch_state_estimation_view/main_dashboard/screen.png`).

## Navigation & Global Shell
- [ ] Build a persistent top navigation window (brand title, Dashboard/Settings/Logs links, Connected pill, antenna icon) that sits above the dockspace.
- [ ] Introduce shared button styles for pills (primary blue, secondary slate) so nav and scene controls stay consistent.
- [ ] Refresh window padding/margins to create the same breathing room as the mock (balanced gutters around the dock space).

## Visual Theme & Typography
- [ ] Extend `ApplyAppTheme` with the mock’s color palette (#111618 cards, #283339 chips, #1193d4 accent, softer borders, muted text).
- [ ] Load Space Grotesk (or similar geometric font) plus Material Symbols in ImGui and apply to headers, body copy, and icon buttons.
- [ ] Set baseline font sizes for headings, sublabels, and monospace data chips so the hierarchy mirrors the vision.

## Card & Layout Infrastructure
- [ ] Replace `ImGuiWindowFlags_AlwaysAutoResize` in panels with responsive min sizes and consistent padding.
- [ ] Add a reusable “Card” helper (rounded background, border, header row, optional badge slot) to wrap panel content.
- [ ] Configure the docking splits so the layout matches the mock’s 8/4 top row and 3-up lower row.

## Flight Scene Card
- [ ] Restyle the 3D scene window background/border to match the mock’s dark gradient card.
- [ ] Embed Rotate/Pan/Zoom pill buttons inside a footer bar within the scene card.
- [ ] Explore subtle vignette/gradient overlay to mirror the rendered mock atmosphere.

## Rotor & Power Panels
- [ ] Rotor Panel: render paired RPM/thrust bars with fixed-width columns, add “Real-time +2%” style badge, and provide per-rotor labels.
- [ ] Power Panel: upgrade the line chart (gradient area + dots) and add “Last 5 Minutes” trend text with delta arrow.
- [ ] Ensure both panels share consistent heading size, spacing, and value typography.

## State Estimation & Control
- [ ] Convert estimator/telemetry data into stacked value chips (rounded slate background, monospace figures).
- [ ] Add compass/icon callouts similar to the mock’s explore and sensors icons.
- [ ] Control Panel: present PID gains with inline sliders and mode badge; include on-card summary values.

## Sensor & INS Data
- [ ] Replace tables with flex-like value rows using chips for each axis reading.
- [ ] Add bottom icon (antenna/wireless) with accent color to mirror mock.
- [ ] Highlight critical readings with colored deltas when thresholds are exceeded.

## Polish & Feedback
- [ ] Audit spacing, corner radius, and font weights across all panels for uniformity.
- [ ] Add hover states (slightly lighter slate) for interactive chips/buttons.
- [ ] Capture before/after screenshots for review and adjust based on usability feedback.

