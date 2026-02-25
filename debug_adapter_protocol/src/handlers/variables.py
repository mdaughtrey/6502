"""Handler for variables requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class VariablesHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        var_ref = args.get("variablesReference", 1)
        if self.session:
            vars_list = self.session.get_variables(var_ref)
        else:
            vars_list = []
        return self.create_response(request, body={"variables": vars_list})
