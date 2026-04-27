"""Backend session management for debug adapter connections."""

import threading
from typing import Any, Dict, List
from urllib import response
from events import StoppedEvent
from enum import Enum

from events.event_queue import EventQueue
from seq_generator import SeqGenerator
from serial_listener import SerialListener

import logging

from handlers.logging_config import logger

class BackendState(Enum):
    """Enum for tracking the state of a debug session."""
    DISCONNECTED = "disconnected"
    RUNNING = "running"
    INITIALIZED = "initialized",
    BREAKPOINT = "breakpoint"

class BackendSession:
    """Manages state for an active debug adapter session/connection.
    
    Each client connection has one session instance that tracks:
    - Sequence number generator for unique message IDs
    - Event queue for async events
    - Session state (initialized, running, stopped, etc.)
    """
    
    def __init__(self, serial_conn=None):
        """Initialize a new session.

        Args:
            serial_conn: Optional open serial connection (pyserial Serial instance)
                         that handlers and backend code can use to talk to the target.
        """
        self.seq_generator = SeqGenerator(start=1)
        self.event_queue = EventQueue(maxsize=1000)
        self.initialized = False
        self.configurating_done = False
        self.running = False
        self.serial_conn = serial_conn
        self.serial_listener = None
        self.state = BackendState.DISCONNECTED

        # In-memory debug state used by request handlers.
        self._next_bp_id = 1
        self.breakpoints: Dict[str, List[Dict[str, Any]]] = {}
        self.threads: Dict[int, Dict[str, Any]] = {1: {"id": 1, "name": "MainThread"}}
        self.variables: Dict[int, List[Dict[str, Any]]] = {1: []}

        # Internal synchronization for step/run completion waits.
        self._sync_cv = threading.Condition()
        self._sync_token = 0
        self._last_completed_sync_token = 0
        self._last_sync_internal: Dict[str, Any] | None = None

        self.logger = logger
        # if self.serial_conn is not None:
        #     try:
        #         self.serial_conn.write(b'xdi')
        #     except Exception as exc:
        #         self.logger.warning(f"Failed to send initial target command: {exc}", exc_info=True)

    def target_write(self, message: bytes) -> None:
        """Write a message to the target device via the serial connection."""
        if self.serial_conn is not None:
            self.logger.info(f"Writing to target: {message}")
            self.serial_conn.write(message)
        
    def mark_initialized(self) -> None:
        """Mark that initialize request has been completed."""
        self.initialized = True
    
    def mark_configuration_done(self) -> None:
        """Mark that configuration is complete."""
        self.configurating_done = True
        self.state = BackendState.RUNNING
    
    def queue_event(self, event: dict) -> None:
        """Queue an event to be sent to the client.
        
        Args:
            event: DAP event dict with seq, type='event', event name, body
        """
        internal_event = self._build_internal_event(event)
        if internal_event is not None:
            self._handle_internal_event(internal_event)
            return

        if dap_event := self._build_dap_event(event):
            self.event_queue.put(dap_event)
        # elif internal_event is None:
        #     self.logger.warning(f"Received event with unknown type: {event.get('event')}")

    def _build_internal_event(self, event: dict) -> Dict[str, Any] | None:
        """Translate target event into an internal synchronization signal."""
        # if event.get("event") == "break":
        #     return {
        #         "kind": "execution_stopped",
        #         "reason": event.get("reason"),
        #         "raw": event,
        #     }
        if event.get("event") == "pinstatus":
            value = int(event.get("pins"), 16)
            return {
                "kind": "pin_status",
                "address": value & 0xFFFF,
                "data": (value >> 40) & 0xFF,
            }
        if event.get("kind") == "memory_dump":
            return event

        return None

    def set_breakpoints(self, breakpoints: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Store breakpoints for a source and return DAP-compatible entries."""
        source = breakpoints.get("source")
        path = source.get("name", "<unknown>")
        self.target_write(b'B') # Clear all breakpoints
        stored = []
        for line in breakpoints["lines"]:
            stored_bp = {
                "id": self._next_bp_id,
                "verified": True,
                "line": line,
            }
            self.target_write(b'b' + f"{line:04x}".encode() + b'\r')
            self._next_bp_id += 1
            stored.append(stored_bp)

        self.breakpoints[path] = stored
        return stored

    def get_threads(self) -> List[Dict[str, Any]]:
        """Return known debuggee threads."""
        return list(self.threads.values())

    def get_stacktrace(self, thread_id: int, start_frame: int = 0, levels: int = 20) -> List[Dict[str, Any]]:
        """Return a placeholder stack trace frame for the active thread."""
        if self.state == BackendState.BREAKPOINT:
            frames = [{"id": 1, "name": "main", "line": self.address, "column": 0 }]
            return frames

    def get_scopes(self, frame_id: int) -> List[Dict[str, Any]]:
        """Return placeholder scopes for a stack frame."""
        return [{"name": "Local", "variablesReference": 1, "expensive": False}]

    def get_variables(self, variables_reference: int) -> List[Dict[str, Any]]:
        """Return variables for a scope reference."""
        return self.variables.get(variables_reference, [])

    def set_variable(self, variables_reference: int, name: str, value: Any) -> Dict[str, Any]:
        """Update or create a variable in the in-memory variable table."""
        vars_list = self.variables.setdefault(variables_reference, [])
        for var in vars_list:
            if var.get("name") == name:
                var["value"] = str(value)
                return var

        new_var = {"name": name, "value": str(value), "variablesReference": 0}
        vars_list.append(new_var)
        return new_var

    def start_serial_listener(self) -> bool:
        """Start background serial listener if a serial connection exists."""
        if self.serial_conn is None:
            self.logger.debug("Serial listener start skipped: no serial connection")
            return False

        if self.serial_listener is None:
            self.serial_listener = SerialListener(
                serial_conn=self.serial_conn,
                queue_event=self.queue_event,
                logger=self.logger,
            )

        started = self.serial_listener.start()
        if started:
            self.logger.debug("Started serial listener for backend session")
        return started

    def stop_serial_listener(self, timeout: float = 1.0) -> bool:
        """Stop background serial listener if it is running."""
        if self.serial_listener is None:
            return False

        stopped = self.serial_listener.stop(timeout=timeout)
        if stopped:
            self.logger.debug("Stopped serial listener for backend session")
        return stopped

    def close(self) -> None:
        """Best-effort cleanup for session resources."""
        try:
            self.stop_serial_listener()
        except Exception:
            self.logger.exception("Failed while stopping serial listener during session close")

    def _build_stopped_event(self, event) -> dict:
        event_msg = {
                "type": "event",
                "event": "stopped",
                "body": StoppedEvent(
                    reason="breakpoint",
                    threadId=1,
                    hitBreakpointIds=[int(event.get("address", 0), 16)],
                    description="hello there i am a description.",
                    text="and i am some text.",
                    allThreadsStopped=True
                ).__dict__
            }
        self.state = BackendState.BREAKPOINT
        self.address = event_msg["body"]["hitBreakpointIds"][0]
        return event_msg

    def _build_dap_event(self, event: dict) -> None:
        """Send a DAP event to the client."""
        dap_map = {
            "break": self._build_stopped_event
        }
        dap_event = dap_map.get(event.get("event"))
        return dap_event(event) if dap_event else None

    def _begin_sync_wait(self) -> int:
        with self._sync_cv:
            self._sync_token += 1
            token = self._sync_token
            self._last_sync_internal = None
            return token

    def _handle_internal_event(self, internal_event: Dict[str, Any]) -> None:
        """Apply internal synchronization updates from target events."""
        if internal_event.get("kind") not in ["pin_status", "memory_dump"]:
            return

        with self._sync_cv:
            self._last_completed_sync_token = self._sync_token
            self._last_sync_internal = internal_event
            self._sync_cv.notify_all()

    def _wait_for_sync_completion(self, token: int, timeout: float = 1.0) -> tuple[bool, Dict[str, Any] | None]:
        """Wait for the target to report a stop corresponding to the issued step."""
        with self._sync_cv:
            completed = self._sync_cv.wait_for(
                lambda: self._last_completed_sync_token >= token,
                timeout=timeout,
            )
            if not completed:
                return False, None
            return True, self._last_sync_internal

    def step_in(self, wait_for_stop: bool = True, timeout: float = 1.0) -> Dict[str, Any]:
        """Handle a 'next' request by resuming execution."""
        if self.serial_conn is None:
            return {"ok": False, "reason": "no-serial-connection"}

        if self.state == BackendState.BREAKPOINT:
            token = self._begin_sync_wait()
            self.target_write(b's')    # Step once, get pin status
            self.target_write(b'p')    # Step once, get pin status
            self.state = BackendState.RUNNING
            if not wait_for_stop:
                return {"ok": True, "awaited": False}

            completed, internal = self._wait_for_sync_completion(token, timeout=timeout)
            if not completed:
                return {"ok": False, "reason": "step-timeout"}
            self.logger.info(f"Step at line {internal['address']:04x}")
            self.queue_event({"event": "break", "address": f"{internal['address']:04x}"})

            return {"ok": True, "awaited": True, "internal": internal}
        elif self.state == BackendState.RUNNING:
            self.target_write(b'r')    # Run
            return {"ok": True, "awaited": False}

        return {"ok": False, "reason": f"invalid-state:{self.state.value}"}

    def _build_read_memory_response(self, response):
        return { "address": response.get("address"),
                    "data": response.get("data")
        }

    def next(self, wait_for_stop: bool = True, timeout: float = 1.0) -> Dict[str, Any]:
        """Handle a 'next' request by resuming execution."""
        if self.serial_conn is None:
            return {"ok": False, "reason": "no-serial-connection"}

        if self.state == BackendState.BREAKPOINT:
            token = self._begin_sync_wait()
            self.target_write(b'i')    # Step once, get pin status
            self.target_write(b'p')    # Step once, get pin status
            self.state = BackendState.RUNNING
            if not wait_for_stop:
                return {"ok": True, "awaited": False}

            completed, internal = self._wait_for_sync_completion(token, timeout=timeout)
            if not completed:
                return {"ok": False, "reason": "step-timeout"}
            self.logger.info(f"Step at line {internal['address']:04x}")
            self.queue_event({"event": "break", "address": f"{internal['address']:04x}"})

            return {"ok": True, "awaited": True, "internal": internal}
        elif self.state == BackendState.RUNNING:
            self.target_write(b'r')    # Run
            return {"ok": True, "awaited": False}

        return {"ok": False, "reason": f"invalid-state:{self.state.value}"}

    def read_memory(self, address: int, length: int, timeout: float = 20.0) -> bytes:
        """Read memory from the target device via the serial connection."""
        if self.serial_conn is None:
            raise RuntimeError("No serial connection available for memory read")

        token = self._begin_sync_wait()
        # Send memory read command (example format, adjust as needed)
        self.target_write(f'd{address:04x}/{length:04x}\r\n'.encode())
        completed, response = self._wait_for_sync_completion(token, timeout=timeout)

        # Read response (this is a placeholder, adjust parsing as needed)
        # response = self.serial_conn.readline().strip()
        self.logger.info(f"Received memory read response: {response}")
        return self._build_read_memory_response(response)

    def target_run(self) -> None:    
        self.target_write(b'c10\r\n')

    def target_continue(self) -> None:    
        self.target_write(b'c10\r\n')

    def target_pause(self) -> None:    
        self.target_write(b'C')

