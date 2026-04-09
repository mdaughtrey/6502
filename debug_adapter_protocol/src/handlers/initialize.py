"""Handler for initialize requests."""

from typing import Any, Dict
from .base_handler import BaseHandler
from events import InitializedEvent

class InitializeHandler(BaseHandler):
    """Handles initialize requests."""
    
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Handle an initialize request.
        
        The initialize request is sent as the first request from the client
        to configure the debug adapter with client capabilities. The adapter
        responds with its capabilities, and then sends the 'initialized' event
        to signal it's ready for configuration requests.
        """
        # Declare adapter capabilities
        capabilities = {
            "supportsConfigurationDoneRequest": True,
            "supportsFunctionBreakpoints": False,
            "supportsConditionalBreakpoints": False,
            "supportsHitConditionalBreakpoints": False,
            "supportsEvaluateForHovers": False,
            "supportsStepBack": False,
            "supportsSetVariable": True,
            "supportsRestartFrame": False,
            "supportsGotoTargetsRequest": True,
            "supportsStepInTargetsRequest": True,
            "supportsCompletionsRequest": False,
            "supportsModulesRequest": False,
            "supportsRestartRequest": True,
            "supportsExceptionOptions": False,
            "supportsValueFormattingOptions": False,
            "supportsExceptionInfoRequest": False,
            "supportTerminateDebuggee": True,
            "supportsDelayedStackTraceLoading": False,
            "supportsLoadedSourcesRequest": False,
            "supportsLogPoints": False,
            "supportsTerminateThreadsRequest": False,
            "supportsSetExpression": False,
            "supportsTerminateRequest": True,
            "supportsDataBreakpoints": False,
            "supportsReadMemoryRequest": True,
            "supportsWriteMemoryRequest": True,
            "supportsDisassembleRequest": False,
            "supportsCancelRequest": False,
            "supportsBreakpointLocationsRequest": True,
        }

        self.backend_session.target_write(b'xxdicu')  # Send initial command to target to indicate we're ready (optional, depends on target protocol)
        # After sending initialize response, send initialized event to signal ready for configuration
        initialized_event = InitializedEvent(seq=0).__dict__  # seq will be assigned by adapter
        
        return self.create_response(request, body=capabilities),  [initialized_event]
