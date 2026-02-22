"""Handler for setBreakpoints requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class SetBreakpointsHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        source = args.get("source", {})
        breakpoints = args.get("breakpoints", [])

        if self.session:
            stored = self.session.set_breakpoints(source, breakpoints)
        else:
            stored = [{"id": i + 1, "verified": True, "line": bp.get("line")} for i, bp in enumerate(breakpoints)]

        return self.create_response(request, body={"breakpoints": stored}), []
