"""Handler for configurationDone requests."""

from typing import Any, Dict
from .base_handler import BaseHandler


class ConfigurationDoneHandler(BaseHandler):
    """Handles configurationDone requests."""
    
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle a configurationDone request.
        
        This request indicates the client has finished initialization.
        """
        return self.create_response(request), []
