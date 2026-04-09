"""Backend session management for debug adapter connections."""

from typing import Any, Dict, List

from events.event_queue import EventQueue
from seq_generator import SeqGenerator
from serial_listener import SerialListener

import logging

from handlers.logging_config import logger



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

        # In-memory debug state used by request handlers.
        self._next_bp_id = 1
        self.breakpoints: Dict[str, List[Dict[str, Any]]] = {}
        self.threads: Dict[int, Dict[str, Any]] = {1: {"id": 1, "name": "MainThread"}}
        self.variables: Dict[int, List[Dict[str, Any]]] = {1: []}

        self.logger = logger
        # if self.serial_conn is not None:
        #     try:
        #         self.serial_conn.write(b'xdi')
        #     except Exception as exc:
        #         self.logger.warning(f"Failed to send initial target command: {exc}", exc_info=True)

    def target_write(self, message: bytes) -> None:
        """Write a message to the target device via the serial connection."""
        if self.serial_conn is not None:
            self.logger.debug(f"Writing to target: {message}")
            self.serial_conn.write(message)
        
    def mark_initialized(self) -> None:
        """Mark that initialize request has been completed."""
        self.initialized = True
    
    def mark_configuration_done(self) -> None:
        """Mark that configuration is complete."""
        self.configurating_done = True
    
    def queue_event(self, event: dict) -> None:
        """Queue an event to be sent to the client.
        
        Args:
            event: DAP event dict with seq, type='event', event name, body
        """
        self.event_queue.put(event)

    def set_breakpoints(self, breakpoints: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Store breakpoints for a source and return DAP-compatible entries."""
        path = source.get("path") or source.get("name") or "<unknown>"
        stored = []
        for bp in breakpoints:
            stored_bp = {
                "id": self._next_bp_id,
                "verified": True,
                "line": bp.get("line"),
            }
            self._next_bp_id += 1
            stored.append(stored_bp)

        self.breakpoints[path] = stored

                stored = []
        for ii, line in enumerate(args.get("lines", [])):
            self.backend_session.target_write(b'b' + f"{line:04x}".encode() + b'\r')
            stored.append({"id": ii + 1, "verified": True, "line": line})

        return stored

    def get_threads(self) -> List[Dict[str, Any]]:
        """Return known debuggee threads."""
        return list(self.threads.values())

    def get_stacktrace(self, thread_id: int, start_frame: int = 0, levels: int = 20) -> List[Dict[str, Any]]:
        """Return a placeholder stack trace frame for the active thread."""
                # if self.session:
        #     frames = self.session.get_stacktrace(thread_id)
        # else:
        frames = [{"id": 1, "name": "main", "line": 0xc200, "column": 0 }]

        return [
            {
                "id": 1,
                "name": "main",
                "line": 1,
                "column": 1,
                "source": {"name": "<program>", "path": "<program>"},
            }
        ]

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
