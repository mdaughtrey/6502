"""Simple in-memory debug session backend used by handlers.

This is a lightweight placeholder implementing minimal state and
operations so handlers can perform meaningful work for tests and
editor interactions.
"""
from typing import Dict, List, Any


class DebugSession:
    def __init__(self, serial_port: str = None):
        self.serial_port = serial_port
        self._next_bp_id = 1
        self.breakpoints: Dict[str, List[Dict[str, Any]]] = {}
        self.threads: Dict[int, Dict[str, Any]] = {1: {"id": 1, "name": "MainThread"}}
        self.variables: Dict[int, List[Dict[str, Any]]] = {1: []}

    def set_breakpoints(self, source: Dict[str, Any], breakpoints: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        path = source.get("path") or source.get("name") or "<unknown>"
        stored = []
        for bp in breakpoints:
            stored_bp = {
                "id": self._next_bp_id,
                "verified": True,
                "line": bp.get("line")
            }
            self._next_bp_id += 1
            stored.append(stored_bp)

        self.breakpoints[path] = stored
        return stored

    def get_threads(self) -> List[Dict[str, Any]]:
        return list(self.threads.values())

    def get_stacktrace(self, thread_id: int, start_frame=0, levels=20) -> List[Dict[str, Any]]:
        # Return a single dummy frame
        return [
            {
                "id": 1,
                "name": "main",
                "line": 1,
                "column": 1,
                "source": {"name": "<program>", "path": "<program>"}
            }
        ]

    def get_scopes(self, frame_id: int) -> List[Dict[str, Any]]:
        return [
            {"name": "Local", "variablesReference": 1, "expensive": False}
        ]

    def get_variables(self, variables_reference: int) -> List[Dict[str, Any]]:
        return self.variables.get(variables_reference, [])

    def set_variable(self, variables_reference: int, name: str, value: Any) -> Dict[str, Any]:
        # Find variable by name and update or create
        vars_list = self.variables.setdefault(variables_reference, [])
        for v in vars_list:
            if v.get("name") == name:
                v["value"] = str(value)
                return v
        new_v = {"name": name, "value": str(value), "variablesReference": 0}
        vars_list.append(new_v)
        return new_v
