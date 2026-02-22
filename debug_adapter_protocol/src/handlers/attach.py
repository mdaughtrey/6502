"""Handler for attach requests."""
from typing import Any, Dict
from .base_handler import BaseHandler
class AttachHandler(BaseHandler):
    """Handles attach requests."""
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle an attach request."""
        return self.create_response(request), []
