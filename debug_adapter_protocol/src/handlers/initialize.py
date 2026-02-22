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
            # "supportsConfigurationDoneRequest": True,
            # "supportsFunctionBreakpoints": True,
            # "supportsConditionalBreakpoints": True,
            # "supportsHitConditionalBreakpoints": True,
            # "supportsEvaluateForHovers": True,
            # "supportsStepBack": False,
            # "supportsSetVariable": True,
            # "supportsRestartFrame": True,
            # "supportsGotoTargetsRequest": False,
            # "supportsStepInTargetsRequest": False,
            # "supportsCompletionsRequest": False,
            # "supportsModulesRequest": False,
            "supportsRestartRequest": True,
            # "supportsExceptionOptions": False,
            # "supportsValueFormattingOptions": True,
            # "supportsExceptionInfoRequest": True,
            "supportTerminateDebuggee": True,
            # "supportsDelayedStackTraceLoading": False,
            # "supportsLoadedSourcesRequest": False,
            # "supportsLogPoints": False,
            # "supportsTerminateThreadsRequest": False,
            # "supportsSetExpression": False,
            # "supportsTerminateRequest": True,
            # "supportsDataBreakpoints": False,
            "supportsReadMemoryRequest": False,
            "supportsWriteMemoryRequest": False,
            # "supportsDisassembleRequest": False,
            # "supportsCancelRequest": True,
            # "supportsBreakpointLocationsRequest": False,
        }

        # After sending initialize response, send initialized event to signal ready for configuration
        initialized_event = InitializedEvent(seq=0).__dict__  # seq will be assigned by adapter
        
        return self.create_response(request, body=capabilities), [initialized_event]
