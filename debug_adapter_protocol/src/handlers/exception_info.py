"""Handler for exceptionInfo requests."""
from typing import Any, Dict
from .base_handler import BaseHandler

class ExceptionInfoHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        return self.create_response(request, body={"exceptionId": "", "breakMode": "never"}), []
