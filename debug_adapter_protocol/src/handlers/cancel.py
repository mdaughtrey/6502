"""Handler for cancel requests."""

from typing import Any, Dict
from .base_handler import BaseHandler


class CancelHandler(BaseHandler):
    """Handles cancel requests."""
    
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle a cancel request.
        
        Cancel requests are used to indicate that the client is no longer
        interested in the result of a previous request.
        """
        return self.create_response(request), []
