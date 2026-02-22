"""Handler for launch requests."""

from typing import Any, Dict
from .base_handler import BaseHandler
from events.event_queue import EventQueue



class LaunchHandler(BaseHandler):
    """Handles launch requests."""
    
    def handle(self, request: Dict[str, Any], event_queue: EventQueue) -> Dict[str, Any]:
        """Handle a launch request.
        
        This request is sent to start the debuggee with or without debugging.
        """
        return self.create_response(request)
