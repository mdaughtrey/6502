#!/usr/bin/env python3

import json
import re
from pathlib import Path
from typing import Any

import typer

app = typer.Typer()

STACK_START = 0x0100
STACK_END = 0x01FF


MODULE_START_RE = re.compile(r"^(?P<module>\S+\.o):$")
MODULE_CODE_RE = re.compile(r"^\s+CODE\s+Offs=(?P<offs>[0-9A-F]+)\s+Size=(?P<size>[0-9A-F]+)")
CODE_SEGMENT_RE = re.compile(r"^CODE\s+(?P<start>[0-9A-F]{6})\s+[0-9A-F]{6}\s+[0-9A-F]{6}")
CFG_SEGMENT_RE = re.compile(r"^(?P<segment>[A-Za-z0-9_]+):\s+load\s+=\s+\S+,\s+type\s+=\s+\S+(?:,\s+start\s+=\s+\$(?P<start>[0-9A-Fa-f]+))?;?$")
MAP_EXPORT_RE = re.compile(r"(?P<name>[A-Za-z_@.][\w@.]*)\s+(?P<addr>[0-9A-F]{6})")
LST_PROC_RE = re.compile(r"^(?P<offset>[0-9A-F]{6})r\s+\d+\s+\.proc\s+(?P<name>[A-Za-z_@.][\w@.]*)$")
LST_LINE_RE = re.compile(
    r"^(?P<offset>[0-9A-F]{6})r\s+\d+\s+(?P<bytes>(?:[0-9A-F]{2}|rr|xx)(?:\s+(?:[0-9A-F]{2}|rr|xx))*)\s+(?P<text>.+)$"
)
LST_LABEL_RE = re.compile(r"^(?P<offset>[0-9A-F]{6})r\s+\d+\s+(?P<label>[A-Za-z_@.][\w@.]*)[:]\s*(?P<text>.*)$")


def build_segment_ranges(segment_starts: dict[str, int | None]) -> list[tuple[int, int, str]]:
    starts = sorted((start, name) for name, start in segment_starts.items() if start is not None)
    ranges: list[tuple[int, int, str]] = []
    for index, (start, name) in enumerate(starts):
        end = starts[index + 1][0] - 1 if index + 1 < len(starts) else 0xFFFF
        ranges.append((start, end, name))
    return ranges


def resolve_segment(address: int, segment_ranges: list[tuple[int, int, str]]) -> str | None:
    for start, end, name in segment_ranges:
        if start <= address <= end:
            return name
    return None


def format_pinstatus(pins: str, segment_name: str | None = None) -> str:
    value = int(pins, 16)
    address = value & 0xFFFF
    read_write = "R" if value & (1 << 18) else "W"
    ready = 1 if value & (1 << 17) else 0
    clock = 1 if value & (1 << 25) else 0
    bus_enable = 1 if value & (1 << 31) else 0
    reset = 1 if value & (1 << 33) else 0
    irq = 1 if value & (1 << 35) else 0
    nmi = 1 if value & (1 << 36) else 0
    sync = 1 if value & (1 << 39) else 0
    data = (value >> 40) & 0xFF

    stack_tag = ""
    if STACK_START <= address <= STACK_END:
        access_kind = "read" if read_write == "R" else "write"
        stack_tag = f" stack={access_kind}"

    segment_tag = f"{segment_name}" if segment_name is not None else ""

    return (
        f"{segment_tag:<9} {address:04X}: {data:02X} rw={read_write} "
        f"sync={sync} clk={clock} be={bus_enable} ready={ready} "
        f"reset={reset} irq={irq} nmi={nmi} {stack_tag}"
    )


def render_screenlog_line(line: str) -> str:
    stripped = line.strip()
    if not stripped:
        return ""

    if stripped.startswith("{"):
        try:
            payload: Any = json.loads(stripped)
        except json.JSONDecodeError:
            return f"; {stripped}"

        if payload.get("event") == "pinstatus" and "pins" in payload:
            return format_pinstatus(str(payload["pins"]))

        return f"{stripped}"

    return f"; {line.rstrip()}"


def render_screenlog(screenlog_file: Path) -> None:
    for raw_line in screenlog_file.read_text().splitlines():
        rendered = render_screenlog_line(raw_line)
        if rendered:
            typer.echo(rendered)
        else:
            typer.echo()


def add_unique_label(labels: dict[int, list[str]], address: int, label: str) -> None:
    label_list = labels.setdefault(address, [])
    if label not in label_list:
        label_list.append(label)


def format_disassembly_line(
    address: int,
    disassembly_text: str,
    routine_starts: dict[int, str],
) -> str:
    routine_address = max((start for start in routine_starts if start <= address), default=None)
    if routine_address is None:
        return f"${address:04X} {disassembly_text}"

    routine_name = routine_starts[routine_address]
    routine_offset = address - routine_address
    return f"{routine_name}+{routine_offset:04X} {disassembly_text}"


def parse_map_file(map_file: Path) -> tuple[int | None, dict[str, int]]:
    code_start: int | None = None
    module_offsets: dict[str, int] = {}
    current_module: str | None = None
    in_exports_by_name = False

    for raw_line in map_file.read_text().splitlines():
        stripped = raw_line.strip()

        if stripped == "Exports list by name:":
            in_exports_by_name = True
            current_module = None
            continue

        if stripped == "Exports list by value:":
            in_exports_by_name = False

        if in_exports_by_name:
            continue

        module_match = MODULE_START_RE.match(raw_line)
        if module_match:
            current_module = module_match.group("module")
            continue

        if raw_line.startswith("Segment list:") or raw_line.startswith("Segment List:"):
            current_module = None

        code_segment_match = CODE_SEGMENT_RE.match(raw_line)
        if code_segment_match:
            code_start = int(code_segment_match.group("start"), 16)
            continue

        if current_module is not None:
            module_code_match = MODULE_CODE_RE.match(raw_line)
            if module_code_match:
                module_offsets[current_module] = int(module_code_match.group("offs"), 16)

    return code_start, module_offsets


def parse_map_exports(map_file: Path) -> dict[int, list[str]]:
    labels: dict[int, list[str]] = {}
    in_exports_by_name = False

    for raw_line in map_file.read_text().splitlines():
        stripped = raw_line.strip()
        if stripped == "Exports list by name:":
            in_exports_by_name = True
            continue

        if stripped == "Exports list by value:":
            break

        if not in_exports_by_name:
            continue

        for match in MAP_EXPORT_RE.finditer(raw_line):
            address = int(match.group("addr"), 16)
            labels.setdefault(address, []).append(match.group("name"))

    return labels


def parse_cfg_file(cfg_file: Path) -> dict[str, int | None]:
    segment_starts: dict[str, int | None] = {}
    in_segments_block = False

    for raw_line in cfg_file.read_text().splitlines():
        stripped = raw_line.strip()
        if stripped == "SEGMENTS {":
            in_segments_block = True
            continue

        if in_segments_block and stripped == "}":
            break

        if not in_segments_block:
            continue

        segment_match = CFG_SEGMENT_RE.match(stripped)
        if segment_match:
            segment_name = segment_match.group("segment")
            start_text = segment_match.group("start")
            segment_starts[segment_name] = int(start_text, 16) if start_text is not None else None

    return segment_starts


def parse_lst_file(
    lst_file: Path,
    module_base_address: int,
) -> tuple[dict[int, str], dict[int, list[str]], dict[int, str]]:
    disassembly: dict[int, str] = {}
    labels: dict[int, list[str]] = {}
    routine_starts: dict[int, str] = {}
    in_code_segment = False

    for raw_line in lst_file.read_text().splitlines():
        normalized_line = raw_line.lower()

        if '.segment "code"' in normalized_line or '.segment code' in normalized_line:
            in_code_segment = True
            continue

        if '.segment "' in normalized_line and '.segment "code"' not in normalized_line:
            in_code_segment = False
            continue

        if not in_code_segment:
            continue

        proc_match = LST_PROC_RE.match(raw_line)
        if proc_match:
            offset = int(proc_match.group("offset"), 16)
            proc_name = proc_match.group("name")
            routine_starts[module_base_address + offset] = proc_name
            add_unique_label(labels, module_base_address + offset, proc_name)
            continue

        label_match = LST_LABEL_RE.match(raw_line)
        if label_match:
            offset = int(label_match.group("offset"), 16)
            label_text = label_match.group("label")
            add_unique_label(labels, module_base_address + offset, label_text)
            continue

        line_match = LST_LINE_RE.match(raw_line)
        if not line_match:
            continue

        offset = int(line_match.group("offset"), 16)
        byte_text = line_match.group("bytes")
        assembly_text = line_match.group("text").rstrip()
        disassembly[module_base_address + offset] = (
            f"{lst_file.name.removesuffix('.lst')}: {byte_text} {assembly_text}"
        )

    return disassembly, labels, routine_starts


def build_ld65_lookup(
    project_dir: Path,
    project_name: str,
) -> tuple[dict[int, str], dict[int, list[str]], dict[int, str]]:
    map_file = project_dir / f"{project_name}.map"
    code_start, module_offsets = parse_map_file(map_file)
    if code_start is None:
        return {}, {}, {}

    disassembly: dict[int, str] = {}
    labels: dict[int, list[str]] = {}
    routine_starts: dict[int, str] = {}
    if map_file.exists():
        for address, label_names in parse_map_exports(map_file).items():
            for label_name in label_names:
                add_unique_label(labels, address, label_name)
    for lst_file in sorted(project_dir.glob("*.lst")):
        module_name = lst_file.name.removesuffix(".asm.lst").removesuffix(".lst") + ".o"
        module_offset = module_offsets.get(module_name)
        if module_offset is None:
            continue

        module_base_address = code_start + module_offset
        file_disassembly, file_labels, file_routine_starts = parse_lst_file(lst_file, module_base_address)
        disassembly.update(file_disassembly)
        for address, label_names in file_labels.items():
            for label_name in label_names:
                add_unique_label(labels, address, label_name)
        routine_starts.update(file_routine_starts)

    return disassembly, labels, routine_starts


def render_ld65_project(
    project_dir: Path,
    project_name: str,
) -> tuple[dict[int, str], dict[int, list[str]], dict[int, str], list[tuple[int, int, str]]]:
    cfg_file = project_dir / f"{project_name}.cfg"
    map_file = project_dir / f"{project_name}.map"
    lst_files = sorted(project_dir.glob("*.lst"))
    cfg_segments = parse_cfg_file(cfg_file) if cfg_file.exists() else {}
    map_code_start, _ = parse_map_file(map_file) if map_file.exists() else (None, {})

    if cfg_file.exists():
        typer.echo(f"; ld65 cfg: {cfg_file}")
        for segment_name, segment_start in cfg_segments.items():
            if segment_start is None:
                typer.echo(f";   {segment_name}: start=<unspecified>")
            else:
                typer.echo(f";   {segment_name}: start=${segment_start:04X}")
    else:
        typer.echo(f"; missing ld65 cfg: {cfg_file}")

    if map_file.exists():
        typer.echo(f"; ld65 map: {map_file}")
        if map_code_start is not None:
            typer.echo(f";   CODE start=${map_code_start:04X}")
        cfg_code_start = cfg_segments.get("CODE")
        if cfg_code_start is not None and map_code_start is not None and cfg_code_start != map_code_start:
            typer.echo(
                f";   warning: cfg CODE start=${cfg_code_start:04X} does not match map CODE start=${map_code_start:04X}"
            )
    else:
        typer.echo(f"; missing ld65 map: {map_file}")

    if lst_files:
        typer.echo("; ld65 lst files:")
        for lst_file in lst_files:
            typer.echo(f";   {lst_file}")
    else:
        typer.echo(f"; no ld65 lst files found in {project_dir}")

    disassembly, labels, routine_starts = build_ld65_lookup(project_dir, project_name)
    segment_ranges = build_segment_ranges(cfg_segments)
    return disassembly, labels, routine_starts, segment_ranges


def render_pinstatus(
    payload: dict[str, Any],
    disassembly: dict[int, str],
    labels: dict[int, list[str]],
    routine_starts: dict[int, str],
    segment_ranges: list[tuple[int, int, str]],
    last_disassembly_address: int | None,
) -> int | None:
    pins = str(payload["pins"])
    value = int(pins, 16)
    address = value & 0xFFFF
    sync = bool(value & (1 << 39))
    read_write = bool(value & (1 << 18))
    clock = bool(value & (1 << 25))
    segment_name = resolve_segment(address, segment_ranges)
    segment_tag = segment_name if segment_name is not None else "UNKNOWN"

    if not clock:
        return last_disassembly_address

    typer.echo(format_pinstatus(pins, segment_name))

    if sync and read_write and address != last_disassembly_address:
        for label in dict.fromkeys(labels.get(address, [])):
            typer.echo(f"{segment_tag:<9} {label}:")
        matched = disassembly.get(address)
        if matched is not None:
            typer.echo(f"{segment_tag:<9} {format_disassembly_line(address, matched, routine_starts)}")
            return address

    return last_disassembly_address


def render_screenlog_with_lookup(
    screenlog_file: Path,
    disassembly: dict[int, str],
    labels: dict[int, list[str]],
    routine_starts: dict[int, str],
    segment_ranges: list[tuple[int, int, str]],
) -> None:
    last_disassembly_address: int | None = None

    for raw_line in screenlog_file.read_text().splitlines():
        stripped = raw_line.strip()
        if not stripped:
            typer.echo()
            continue

        if stripped.startswith("{"):
            try:
                payload: Any = json.loads(stripped)
            except json.JSONDecodeError:
                typer.echo(f"; {stripped}")
                continue

            if payload.get("event") == "pinstatus" and "pins" in payload:
                last_disassembly_address = render_pinstatus(
                    payload,
                    disassembly,
                    labels,
                    routine_starts,
                    segment_ranges,
                    last_disassembly_address,
                )
            else:
                typer.echo(f"; {stripped}")
            continue

        typer.echo(f"; {raw_line.rstrip()}")


@app.command()
def pdb(
    name: str = typer.Argument(
        "postdebug",
        help="Label for the rendered trace",
    ),
    ld65_project_dir: Path | None = typer.Option(
        None,
        "--ld65-project-dir",
        exists=True,
        file_okay=False,
        dir_okay=True,
        readable=True,
        help="Directory containing the ld65 project files",
    ),
    ld65_project_name: str | None = typer.Option(
        None,
        "--ld65-project-name",
        help="ld65 project name used to resolve cfg and map files",
    ),
    screenlog_file: Path = typer.Option(
        ...,
        "--screenlog-file",
        "-s",
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="Path to the input screenlog file",
    ),
):
    typer.echo(f"; {name}")
    disassembly: dict[int, str] = {}
    labels: dict[int, list[str]] = {}
    routine_starts: dict[int, str] = {}
    segment_ranges: list[tuple[int, int, str]] = []
    if ld65_project_dir is not None and ld65_project_name is not None:
        disassembly, labels, routine_starts, segment_ranges = render_ld65_project(ld65_project_dir, ld65_project_name)
    render_screenlog_with_lookup(screenlog_file, disassembly, labels, routine_starts, segment_ranges)

if __name__ == "__main__":
    app()

