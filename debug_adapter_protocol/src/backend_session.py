"""Backend session management for debug adapter connections."""

from events.event_queue import EventQueue
from seq_generator import SeqGenerator

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
        self.serial_conn.write(b'xdi')
        self.logger = logger

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
