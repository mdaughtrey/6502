"""Base handler class for all DAP request handlers."""

from abc import ABC, abstractmethod
from typing import Any, Dict
import logging

from .logging_config import logger


class BaseHandler(ABC):
    """Abstract base class for all DAP request handlers."""
    
    def __init__(self, serial_port: str = None, debug_session=None, backend_session=None):
        """Initialize the handler.
        
        Args:
            serial_port: Optional serial port for the debugger connection.
            debug_session: DebugSession with breakpoints, threads, variables state.
            backend_session: BackendSession with seq_generator and event_queue for this connection.
        """
        self.serial_port = serial_port
        self.debug_session = debug_session
        self.backend_session = backend_session
        # Legacy compatibility
        self.session = debug_session
        self.seq_counter = 0
        self.logger = logger
    
    @abstractmethod
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle a DAP request.
        
        Args:
            request: The DAP request message.
            
        Returns:
            Tuple of (response_dict, events_list) where:
            - response_dict: DAP response message (required)
            - events_list: List of DAP events to send (can be empty)
        """
        pass
    
    def _handle_with_logging(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Internal method that wraps handle() with logging.
        
        Args:
            request: The DAP request message.
            
        Returns:
            Tuple of (response_dict, events_list).
        """
        command = request.get('command', 'unknown')
        seq = request.get('seq', 0)
        self.logger.debug(f"Handler {self.__class__.__name__} processing command '{command}' (seq={seq})")
        
        try:
            response, events = self.handle(request)
            self.logger.debug(f"Handler {self.__class__.__name__} completed command '{command}' (seq={seq}) with success={response.get('success', False)}, {len(events)} events")
            return response, events
        except Exception as e:
            self.logger.error(f"Handler {self.__class__.__name__} failed for command '{command}' (seq={seq}): {str(e)}", exc_info=True)
            raise
    
    def create_response(self, request: Dict[str, Any], success: bool = True, 
                       body: Dict[str, Any] = None, message: str = None) -> Dict[str, Any]:
        """Create a standard DAP response.
        
        Args:
            request: The original request message.
            success: Whether the request was successful.
            body: Optional response body.
            message: Optional error message.
            
        Returns:
            A properly formatted DAP response.
        """
        response = {
            "type": "response",
            # "seq": self._get_next_seq(),
            "request_seq": request.get("seq", 0),
            "success": success,
            "command": request.get("command", ""),
        }
        
        if body:
            response["body"] = body
        
        if message:
            response["message"] = message
            
        return response
    
    def _get_next_seq(self) -> int:
        """Get the next sequence number."""
        self.seq_counter += 1
        return self.seq_counter
