"""Handler for setBreakpoints requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class SetBreakpointsHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        self.backend_session.target_write(b'c') # Clear all breakpoints

        # [self.backend_session.target_write(b'b' + format(int(line), "x").encode() + b'\n') for line in args.get("lines", [])]
        # source = args.get("source", {})
        # breakpoints = args.get("breakpoints", [])
        # for line in args.get("lines", []):
        # if self.session:
        #     stored = self.session.set_breakpoints(source, )
        # else:
        #     stored = [{"id": i + 1, "verified": True, "line": bp.get("line")} for i, bp in enumerate(breakpoints)]
        stored = []
        for ii, line in enumerate(args.get("lines", [])):
            self.backend_session.target_write(b'b' + f"{line:04x}".encode() + b'\r')
            stored.append({"id": ii + 1, "verified": True, "line": line})

        return self.create_response(request, body={"breakpoints": stored}), []
