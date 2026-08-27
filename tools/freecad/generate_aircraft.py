"""Generate reference-aircraft CAD artifacts from aircraft-spec v1."""

import argparse
import hashlib
import json
import os
from pathlib import Path

import FreeCAD as App
import Mesh
import Part


MM_PER_M = 1000.0


def positive(value, name):
    parsed = float(value)
    if parsed <= 0.0:
        raise ValueError(f"{name} must be positive")
    return parsed


def vector3(value, name, positive_only=False):
    if not isinstance(value, list) or len(value) != 3:
        raise ValueError(f"{name} must contain three values")
    parsed = [float(component) for component in value]
    if positive_only and any(component <= 0.0 for component in parsed):
        raise ValueError(f"{name} components must be positive")
    return parsed


def cylinder_between(start, end, radius):
    direction = end.sub(start)
    length = direction.Length
    if length <= 0.0:
        raise ValueError("cylinder endpoints must differ")
    return Part.makeCylinder(radius, length, start, direction.normalize())


def add_feature(document, name, label, shape):
    feature = document.addObject("PartDesign::Feature", name)
    feature.Label = label
    feature.Shape = shape
    return feature


def file_digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def normalize_generated_text(path):
    lines = Path(path).read_text(encoding="utf-8").splitlines()
    Path(path).write_text(
        "\n".join(line.rstrip() for line in lines) + "\n", encoding="utf-8"
    )


def build(spec_path, output_directory):
    spec_path = Path(spec_path).resolve()
    output_directory = Path(output_directory).resolve()
    output_directory.mkdir(parents=True, exist_ok=True)
    spec = json.loads(spec_path.read_text(encoding="utf-8"))

    if spec.get("schema_version") != 1:
        raise ValueError("only aircraft specification version 1 is supported")
    if spec.get("frames") != {
        "inertial": "NED",
        "body": "FRD",
        "quaternion": "body_to_inertial_scalar_first",
    }:
        raise ValueError("the CAD generator requires the NED/FRD v1 frame contract")

    geometry = spec["geometry"]
    body_dimensions = vector3(
        geometry["body"]["dimensions_m"], "body.dimensions_m", True
    )
    arm_radius = positive(
        geometry["arm"]["outer_diameter_m"], "arm.outer_diameter_m"
    ) * MM_PER_M / 2.0
    mount_radius = positive(
        geometry["motor_mount"]["diameter_m"], "motor_mount.diameter_m"
    ) * MM_PER_M / 2.0
    mount_thickness = positive(
        geometry["motor_mount"]["thickness_m"], "motor_mount.thickness_m"
    ) * MM_PER_M
    landing = geometry["landing_gear"]
    skid_length = positive(landing["skid_length_m"], "landing_gear.skid_length_m") * MM_PER_M
    skid_spacing = positive(landing["skid_spacing_m"], "landing_gear.skid_spacing_m") * MM_PER_M
    ground_clearance = positive(
        landing["ground_clearance_m"], "landing_gear.ground_clearance_m"
    ) * MM_PER_M
    landing_radius = positive(
        landing["tube_diameter_m"], "landing_gear.tube_diameter_m"
    ) * MM_PER_M / 2.0

    length, width, height = [value * MM_PER_M for value in body_dimensions]
    rotors = spec.get("rotors", [])
    if not rotors:
        raise ValueError("at least one rotor is required")

    document = App.newDocument("ASRReferenceAircraft")
    document.addObject("App::FeaturePython", "Contract")
    contract = document.getObject("Contract")
    contract.addProperty("App::PropertyString", "AircraftId")
    contract.addProperty("App::PropertyInteger", "AircraftRevision")
    contract.addProperty("App::PropertyString", "CoordinateContract")
    contract.AircraftId = spec["aircraft_id"]
    contract.AircraftRevision = int(spec["revision"])
    contract.CoordinateContract = "FRD metres; exported geometry is millimetres"

    components = []
    body = Part.makeBox(
        length,
        width,
        height,
        App.Vector(-length / 2.0, -width / 2.0, -height / 2.0),
    )
    components.append(add_feature(document, "Body", "Central avionics body", body))

    for index, rotor in enumerate(rotors):
        rotor_id = rotor["id"]
        position_m = vector3(rotor["position_body_m"], f"rotors[{index}].position_body_m")
        position = App.Vector(*(component * MM_PER_M for component in position_m))
        arm_end = App.Vector(position.x, position.y, 0.0)
        arm_shape = cylinder_between(App.Vector(0.0, 0.0, 0.0), arm_end, arm_radius)
        components.append(
            add_feature(document, f"Arm_{index + 1}", f"Arm {rotor_id}", arm_shape)
        )
        mount_shape = Part.makeCylinder(
            mount_radius,
            mount_thickness,
            App.Vector(position.x, position.y, -mount_thickness / 2.0),
            App.Vector(0.0, 0.0, 1.0),
        )
        components.append(
            add_feature(
                document, f"MotorMount_{index + 1}", f"Motor mount {rotor_id}", mount_shape
            )
        )

    skid_z = height / 2.0 + ground_clearance
    for side_index, y in enumerate((-skid_spacing / 2.0, skid_spacing / 2.0), start=1):
        skid_start = App.Vector(-skid_length / 2.0, y, skid_z)
        skid_end = App.Vector(skid_length / 2.0, y, skid_z)
        skid = cylinder_between(skid_start, skid_end, landing_radius)
        components.append(add_feature(document, f"Skid_{side_index}", f"Landing skid {side_index}", skid))
        for x_index, x in enumerate((-length * 0.32, length * 0.32), start=1):
            support = cylinder_between(
                App.Vector(x, y, height / 2.0),
                App.Vector(x, y, skid_z),
                landing_radius,
            )
            components.append(
                add_feature(
                    document,
                    f"LandingSupport_{side_index}_{x_index}",
                    f"Landing support {side_index}.{x_index}",
                    support,
                )
            )

    assembly_shape = Part.makeCompound([component.Shape for component in components])
    assembly = add_feature(document, "AircraftAssembly", "ASR reference aircraft", assembly_shape)
    for component in components:
        component.Visibility = False
    document.recompute()

    stem = spec["aircraft_id"]
    paths = {
        "fcstd": output_directory / f"{stem}.FCStd",
        "step": output_directory / f"{stem}.step",
        "stl": output_directory / f"{stem}.stl",
        "obj": output_directory / f"{stem}.obj",
    }
    # FreeCAD creates timestamped FCBak files when saveAs replaces a document.
    # Generated output must remain a closed, predictable artifact set.
    for backup in output_directory.glob(f"{stem}.*.FCBak"):
        backup.unlink()
    paths["fcstd"].unlink(missing_ok=True)
    document.saveAs(str(paths["fcstd"]))
    Part.export([assembly], str(paths["step"]))
    Mesh.export([assembly], str(paths["stl"]))
    Mesh.export([assembly], str(paths["obj"]))
    normalize_generated_text(paths["step"])
    normalize_generated_text(paths["obj"])

    bounds = assembly_shape.BoundBox
    repository_root = output_directory.parents[1]
    try:
        source_path = str(spec_path.relative_to(repository_root))
    except ValueError:
        source_path = str(spec_path)
    manifest = {
        "schema_version": 1,
        "aircraft_id": spec["aircraft_id"],
        "aircraft_revision": spec["revision"],
        "coordinate_contract": "FRD; source metres; CAD exports millimetres",
        "source": {
            "path": source_path,
            "sha256": file_digest(spec_path),
        },
        "bounds_mm": {
            "minimum": [bounds.XMin, bounds.YMin, bounds.ZMin],
            "maximum": [bounds.XMax, bounds.YMax, bounds.ZMax],
        },
        "artifacts": {
            name: {
                "path": path.name,
                "sha256": file_digest(path),
                "byte_reproducible": name in {"stl", "obj"},
            }
            for name, path in paths.items()
        },
    }
    manifest_path = output_directory / f"{stem}.geometry-manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(manifest_path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--spec", default=os.environ.get("ASR_AIRCRAFT_SPEC"))
    parser.add_argument(
        "--output-directory", default=os.environ.get("ASR_AIRCRAFT_OUTPUT_DIRECTORY")
    )
    arguments = parser.parse_args([] if os.environ.get("ASR_AIRCRAFT_SPEC") else None)
    if not arguments.spec or not arguments.output_directory:
        parser.error("--spec and --output-directory are required")
    build(arguments.spec, arguments.output_directory)


if __name__ == "__main__":
    main()
