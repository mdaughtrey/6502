"""Handler for writeMemory requests."""
from typing import Any, Dict
from .base_handler import BaseHandler
from events.event_queue import EventQueue

class WriteMemoryHandler(BaseHandler):
    def handle(self, request: Dict[str, Any], event_queue: EventQueue) -> Dict[str, Any]:
        return self.create_response(request, body={"bytesWritten": 0})
