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
            "supportsFunctionBreakpoints": True,
            "supportsConditionalBreakpoints": True,
            "supportsHitConditionalBreakpoints": True,
            "supportsEvaluateForHovers": True,
            "supportsStepBack": True,
            "supportsSetVariable": True,
            "supportsRestartFrame": True,
            "supportsGotoTargetsRequest": True,
            "supportsStepInTargetsRequest": True,
            "supportsCompletionsRequest": True,
            "supportsModulesRequest": True,
            "supportsRestartRequest": True,
            "supportsExceptionOptions": True,
            "supportsValueFormattingOptions": True,
            "supportsExceptionInfoRequest": True,
            "supportTerminateDebuggee": True,
            "supportsDelayedStackTraceLoading": True,
            "supportsLoadedSourcesRequest": True,
            "supportsLogPoints": True,
            "supportsTerminateThreadsRequest": True,
            "supportsSetExpression": True,
            "supportsTerminateRequest": True,
            "supportsDataBreakpoints": True,
            "supportsReadMemoryRequest": True,
            "supportsWriteMemoryRequest": True,
            "supportsDisassembleRequest": True,
            "supportsCancelRequest": True,
            "supportsBreakpointLocationsRequest": True,
        }

        # After sending initialize response, send initialized event to signal ready for configuration
        initialized_event = InitializedEvent(seq=0).__dict__  # seq will be assigned by adapter
        
        return self.create_response(request, body=capabilities), [initialized_event]
