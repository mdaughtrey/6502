"""Handler for readMemory requests."""
from typing import Any, Dict
from .base_handler import BaseHandler

class ReadMemoryHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        return self.create_response(request, body={"address": "0x0", "data": ""}), []
