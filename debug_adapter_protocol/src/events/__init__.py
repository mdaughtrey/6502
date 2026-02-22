"""Debug Adapter Protocol Events.

Generated from dap.json schema.
"""

from typing import Optional, Any, List, Dict
from dataclasses import dataclass, field


@dataclass
class InitializedEvent:
    """Event indicating the debug adapter is ready to accept configuration requests."""
    seq: int
    type: str = "event"
    event: str = "initialized"


@dataclass
class StoppedEvent:
    """Event indicating execution of the debuggee has stopped."""
    seq: int
    reason: str
    threadId: int
    type: str = "event"
    event: str = "stopped"
    description: Optional[str] = None
    preserveFocusHint: Optional[bool] = None
    text: Optional[str] = None
    allThreadsStopped: Optional[bool] = None
    hitBreakpointIds: Optional[List[int]] = field(default_factory=list)


@dataclass
class ContinuedEvent:
    """Event indicating the execution of the debuggee has continued."""
    seq: int
    threadId: int
    type: str = "event"
    event: str = "continued"
    allThreadsContinued: Optional[bool] = None


@dataclass
class ExitedEvent:
    """Event indicating the debuggee has exited."""
    seq: int
    exitCode: int
    type: str = "event"
    event: str = "exited"


@dataclass
class TerminatedEvent:
    """Event indicating debugging of the debuggee has terminated."""
    seq: int
    type: str = "event"
    event: str = "terminated"
    restart: Optional[Any] = None


@dataclass
class ThreadEvent:
    """Event indicating a thread has started or exited."""
    seq: int
    reason: str
    threadId: int
    type: str = "event"
    event: str = "thread"


@dataclass
class OutputEvent:
    """Event indicating the target has produced some output."""
    seq: int
    output: str
    type: str = "event"
    event: str = "output"
    category: Optional[str] = None
    group: Optional[str] = None
    variablesReference: Optional[int] = None
    source: Optional[Dict[str, Any]] = None
    line: Optional[int] = None
    column: Optional[int] = None
    data: Optional[Any] = None
    locationReference: Optional[int] = None


@dataclass
class BreakpointEvent:
    """Event indicating information about a breakpoint has changed."""
    seq: int
    reason: str
    breakpoint: Dict[str, Any]
    type: str = "event"
    event: str = "breakpoint"


@dataclass
class ModuleEvent:
    """Event indicating information about a module has changed."""
    seq: int
    reason: str
    module: Dict[str, Any]
    type: str = "event"
    event: str = "module"


@dataclass
class LoadedSourceEvent:
    """Event indicating a source has been added, changed, or removed."""
    seq: int
    reason: str
    source: Dict[str, Any]
    type: str = "event"
    event: str = "loadedSource"


@dataclass
class ProcessEvent:
    """Event indicating the debugger has begun debugging a new process."""
    seq: int
    name: str
    type: str = "event"
    event: str = "process"
    systemProcessId: Optional[int] = None
    isLocalProcess: Optional[bool] = None
    startMethod: Optional[str] = None
    pointerSize: Optional[int] = None


@dataclass
class CapabilitiesEvent:
    """Event indicating one or more capabilities have changed."""
    seq: int
    capabilities: Dict[str, Any]
    type: str = "event"
    event: str = "capabilities"


@dataclass
class ProgressStartEvent:
    """Event signaling that a long running operation is about to start."""
    seq: int
    progressId: str
    title: str
    type: str = "event"
    event: str = "progressStart"
    requestId: Optional[int] = None
    cancellable: Optional[bool] = None
    message: Optional[str] = None
    percentage: Optional[float] = None


@dataclass
class ProgressUpdateEvent:
    """Event signaling progress reporting needs to be updated."""
    seq: int
    progressId: str
    type: str = "event"
    event: str = "progressUpdate"
    message: Optional[str] = None
    percentage: Optional[float] = None


@dataclass
class ProgressEndEvent:
    """Event signaling the end of progress reporting."""
    seq: int
    progressId: str
    type: str = "event"
    event: str = "progressEnd"
    message: Optional[str] = None


@dataclass
class InvalidatedEvent:
    """Event signaling that some state in the debug adapter has changed."""
    seq: int
    type: str = "event"
    event: str = "invalidated"
    areas: Optional[List[str]] = field(default_factory=list)
    threadId: Optional[int] = None
    stackFrameId: Optional[int] = None


@dataclass
class MemoryEvent:
    """Event indicating that some memory range has been updated."""
    seq: int
    memoryReference: str
    offset: int
    count: int
    type: str = "event"
    event: str = "memory"


# Type alias for any event
DAP_Event = (
    InitializedEvent
    | StoppedEvent
    | ContinuedEvent
    | ExitedEvent
    | TerminatedEvent
    | ThreadEvent
    | OutputEvent
    | BreakpointEvent
    | ModuleEvent
    | LoadedSourceEvent
    | ProcessEvent
    | CapabilitiesEvent
    | ProgressStartEvent
    | ProgressUpdateEvent
    | ProgressEndEvent
    | InvalidatedEvent
    | MemoryEvent
)

__all__ = [
    "InitializedEvent",
    "StoppedEvent",
    "ContinuedEvent",
    "ExitedEvent",
    "TerminatedEvent",
    "ThreadEvent",
    "OutputEvent",
    "BreakpointEvent",
    "ModuleEvent",
    "LoadedSourceEvent",
    "ProcessEvent",
    "CapabilitiesEvent",
    "ProgressStartEvent",
    "ProgressUpdateEvent",
    "ProgressEndEvent",
    "InvalidatedEvent",
    "MemoryEvent",
    "DAP_Event",
]
