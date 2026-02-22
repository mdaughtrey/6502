"""Handler for scopes requests."""
from typing import Any, Dict
from .base_handler import BaseHandler



class ScopesHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        frame_id = args.get("frameId", 1)
        if self.session:
            scopes = self.session.get_scopes(frame_id)
        else:
            scopes = [{"name": "Local", "variablesReference": 1}]
        return self.create_response(request, body={"scopes": scopes}), []
