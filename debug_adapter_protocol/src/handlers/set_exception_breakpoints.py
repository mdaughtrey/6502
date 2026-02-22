"""Handler for setExceptionBreakpoints requests."""
from typing import Any, Dict
from .base_handler import BaseHandler

class SetExceptionBreakpointsHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        return self.create_response(request, body={"breakpoints": []}), []
