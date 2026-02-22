"""Handler for continue requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class ContinueHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        # In a real backend this would resume execution. Here we simulate success.
        return self.create_response(request, body={"allThreadsContinued": True}), []
