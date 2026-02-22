"""Handler for variables requests."""
from typing import Any, Dict
from .base_handler import BaseHandler
from events.event_queue import EventQueue



class VariablesHandler(BaseHandler):
    def handle(self, request: Dict[str, Any], event_queue: EventQueue) -> Dict[str, Any]:
        args = request.get("arguments", {})
        var_ref = args.get("variablesReference", 1)
        if self.session:
            vars_list = self.session.get_variables(var_ref)
        else:
            vars_list = []
        return self.create_response(request, body={"variables": vars_list})
