"""Handler for threads requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class ThreadsHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        if self.session:
            threads = self.session.get_threads()
        else:
            threads = [{"id": 1, "name": "MainThread"}]
        
        return self.create_response(request, body={"threads": threads}), []
