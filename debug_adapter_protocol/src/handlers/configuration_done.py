"""Handler for configurationDone requests."""

from typing import Any, Dict
from .base_handler import BaseHandler
from events import StoppedEvent, OutputEvent


class ConfigurationDoneHandler(BaseHandler):
    """Handles configurationDone requests."""
    
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle a configurationDone request.
        
        This request indicates the client has finished initialization.
        """
        try:
            self.backend_session.start_serial_listener()
        except Exception:
            self.logger.exception('Failed to start serial listener')
        self.backend_session.target_run()
        # event = self.create_event(event="stopped",
        #                            body=StoppedEvent(
        #                                reason="breakpoint",
        #                                threadId=1,
        #                                hitBreakpointIds=[1],
        #                                description="hello there i am a description.",
        #                                text="and i am some text.",
        #                                allThreadsStopped=True).__dict__)
        #  event = OutputEvent(category="console", output="Configuration done received. Starting execution...\n", seq=self._get_next_seq()).__dict__

        # self.backend_session.queue_event(event)
        
        return self.create_response(request), []
