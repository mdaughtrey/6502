"""Handler for stackTrace requests."""
from typing import Any, Dict
from .base_handler import BaseHandler



class StackTraceHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        thread_id = args.get("threadId", 1)
        if self.session:
            frames = self.session.get_stacktrace(thread_id)
        else:
            frames = [{"id": 1, "name": "main", "line": 1, "column": 1, "source": {"name": "<program>"}}]
        return self.create_response(request, body={"stackFrames": frames}), []
