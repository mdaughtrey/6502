"""Handler for setVariable requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class SetVariableHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        args = request.get("arguments", {})
        var_ref = args.get("variablesReference", 1)
        name = args.get("name")
        value = args.get("value")

        if self.session:
            updated = self.session.set_variable(var_ref, name, value)
            return self.create_response(request, body={"value": updated.get("value")}), []
        return self.create_response(request, body={"value": str(value)}), []
