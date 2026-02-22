"""Dispatcher for routing DAP requests to appropriate handlers."""

from typing import Any, Dict
import logging

from .logging_config import logger
from backend import DebugSession
from .cancel import CancelHandler
from .initialize import InitializeHandler
from .configuration_done import ConfigurationDoneHandler
from .launch import LaunchHandler
from .attach import AttachHandler
from .restart import RestartHandler
from .disconnect import DisconnectHandler
from .terminate import TerminateHandler
from .breakpoint_locations import BreakpointLocationsHandler
from .set_breakpoints import SetBreakpointsHandler
from .set_function_breakpoints import SetFunctionBreakpointsHandler
from .set_exception_breakpoints import SetExceptionBreakpointsHandler
from .data_breakpoint_info import DataBreakpointInfoHandler
from .set_data_breakpoints import SetDataBreakpointsHandler
from .set_instruction_breakpoints import SetInstructionBreakpointsHandler
from .continue_request import ContinueHandler
from .next_request import NextHandler
from .step_in import StepInHandler
from .step_out import StepOutHandler
from .step_back import StepBackHandler
from .reverse_continue import ReverseContinueHandler
from .restart_frame import RestartFrameHandler
from .goto_request import GotoHandler
from .pause import PauseHandler
from .stack_trace import StackTraceHandler
from .scopes import ScopesHandler
from .variables import VariablesHandler
from .set_variable import SetVariableHandler
from .source import SourceHandler
from .threads import ThreadsHandler
from .terminate_threads import TerminateThreadsHandler
from .modules import ModulesHandler
from .loaded_sources import LoadedSourcesHandler
from .evaluate import EvaluateHandler
from .set_expression import SetExpressionHandler
from .step_in_targets import StepInTargetsHandler
from .goto_targets import GotoTargetsHandler
from .completions import CompletionsHandler
from .exception_info import ExceptionInfoHandler
from .read_memory import ReadMemoryHandler
from .write_memory import WriteMemoryHandler
from .disassemble import DisassembleHandler
from .locations import LocationsHandler
from .run_in_terminal import RunInTerminalHandler
from .start_debugging import StartDebuggingHandler



class Dispatcher:
    """Routes DAP requests to appropriate handlers."""
    
    def __init__(self, serial_port: str = None, backend_session=None):
        """Initialize the dispatcher.
        
        Args:
            serial_port: Optional serial port for the debugger connection.
            backend_session: Per-connection BackendSession (seq_generator, event_queue).
                             If None, handlers will use default state management.
        """
        self.serial_port = serial_port
        self.backend_session = backend_session
        # Create a shared debug session for handlers (breakpoints, threads, variables)
        self.debug_session = DebugSession(serial_port=serial_port)
        self.handlers = {
            'cancel': CancelHandler(serial_port, self.debug_session, self.backend_session),
            'initialize': InitializeHandler(serial_port, self.debug_session, self.backend_session),
            'configurationDone': ConfigurationDoneHandler(serial_port, self.debug_session, self.backend_session),
            'launch': LaunchHandler(serial_port, self.debug_session, self.backend_session),
            'attach': AttachHandler(serial_port, self.debug_session, self.backend_session),
            'restart': RestartHandler(serial_port, self.debug_session, self.backend_session),
            'disconnect': DisconnectHandler(serial_port, self.debug_session, self.backend_session),
            'terminate': TerminateHandler(serial_port, self.debug_session, self.backend_session),
            'breakpointLocations': BreakpointLocationsHandler(serial_port, self.debug_session, self.backend_session),
            'setBreakpoints': SetBreakpointsHandler(serial_port, self.debug_session, self.backend_session),
            'setFunctionBreakpoints': SetFunctionBreakpointsHandler(serial_port, self.debug_session, self.backend_session),
            'setExceptionBreakpoints': SetExceptionBreakpointsHandler(serial_port, self.debug_session, self.backend_session),
            'dataBreakpointInfo': DataBreakpointInfoHandler(serial_port, self.debug_session, self.backend_session),
            'setDataBreakpoints': SetDataBreakpointsHandler(serial_port, self.debug_session, self.backend_session),
            'setInstructionBreakpoints': SetInstructionBreakpointsHandler(serial_port, self.debug_session, self.backend_session),
            'continue': ContinueHandler(serial_port, self.debug_session, self.backend_session),
            'next': NextHandler(serial_port, self.debug_session, self.backend_session),
            'stepIn': StepInHandler(serial_port, self.debug_session, self.backend_session),
            'stepOut': StepOutHandler(serial_port, self.debug_session, self.backend_session),
            'stepBack': StepBackHandler(serial_port, self.debug_session, self.backend_session),
            'reverseContinue': ReverseContinueHandler(serial_port, self.debug_session, self.backend_session),
            'restartFrame': RestartFrameHandler(serial_port, self.debug_session, self.backend_session),
            'goto': GotoHandler(serial_port, self.debug_session, self.backend_session),
            'pause': PauseHandler(serial_port, self.debug_session, self.backend_session),
            'stackTrace': StackTraceHandler(serial_port, self.debug_session, self.backend_session),
            'scopes': ScopesHandler(serial_port, self.debug_session, self.backend_session),
            'variables': VariablesHandler(serial_port, self.debug_session, self.backend_session),
            'setVariable': SetVariableHandler(serial_port, self.debug_session, self.backend_session),
            'source': SourceHandler(serial_port, self.debug_session, self.backend_session),
            'threads': ThreadsHandler(serial_port, self.debug_session, self.backend_session),
            'terminateThreads': TerminateThreadsHandler(serial_port, self.debug_session, self.backend_session),
            'modules': ModulesHandler(serial_port, self.debug_session, self.backend_session),
            'loadedSources': LoadedSourcesHandler(serial_port, self.debug_session, self.backend_session),
            'evaluate': EvaluateHandler(serial_port, self.debug_session, self.backend_session),
            'setExpression': SetExpressionHandler(serial_port, self.debug_session, self.backend_session),
            'stepInTargets': StepInTargetsHandler(serial_port, self.debug_session, self.backend_session),
            'gotoTargets': GotoTargetsHandler(serial_port, self.debug_session, self.backend_session),
            'completions': CompletionsHandler(serial_port, self.debug_session, self.backend_session),
            'exceptionInfo': ExceptionInfoHandler(serial_port, self.debug_session, self.backend_session),
            'readMemory': ReadMemoryHandler(serial_port, self.debug_session, self.backend_session),
            'writeMemory': WriteMemoryHandler(serial_port, self.debug_session, self.backend_session),
            'disassemble': DisassembleHandler(serial_port, self.debug_session, self.backend_session),
            'locations': LocationsHandler(serial_port, self.debug_session, self.backend_session),
            'runInTerminal': RunInTerminalHandler(serial_port, self.debug_session, self.backend_session),
            'startDebugging': StartDebuggingHandler(serial_port, self.debug_session, self.backend_session),
        }
    
    def dispatch(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        """Dispatch a request to the appropriate handler.
        
        Args:
            request: The DAP request message.
            
        Returns:
            Tuple of (response_dict, events_list)
        """
        command = request.get('command')
        
        if command not in self.handlers:
            # Return error response for unknown command
            logger.debug(f"Unknown command received: '{command}'")
            return ({
                'type': 'response',
                'seq': 0,
                'request_seq': request.get('seq', 0),
                'success': False,
                'command': command,
                'message': f'Unknown command: {command}'
            }, [])
        
        try:
            handler = self.handlers[command]
            return handler._handle_with_logging(request)
        except Exception as e:
            logger.error(f"Dispatcher error handling {command}: {str(e)}", exc_info=True)
            return ({
                'type': 'response',
                'seq': 0,
                'request_seq': request.get('seq', 0),
                'success': False,
                'command': command,
                'message': f'Error handling {command}: {str(e)}'
            }, [])
