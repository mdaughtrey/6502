"""Handler for threads requests."""
from typing import Any, Dict
from .base_handler import BaseHandler
from events.event_queue import EventQueue



class ThreadsHandler(BaseHandler):
    def handle(self, request: Dict[str, Any], event_queue: EventQueue) -> Dict[str, Any]:
        if self.session:
            threads = self.session.get_threads()
        else:
            threads = [{"id": 1, "name": "MainThread"}]
        return self.create_response(request, body={"threads": threads})
