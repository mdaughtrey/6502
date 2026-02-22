"""Backend session management for debug adapter connections."""

from events.event_queue import EventQueue
from seq_generator import SeqGenerator


class BackendSession:
    """Manages state for an active debug adapter session/connection.
    
    Each client connection has one session instance that tracks:
    - Sequence number generator for unique message IDs
    - Event queue for async events
    - Session state (initialized, running, stopped, etc.)
    """
    
    def __init__(self):
        """Initialize a new session."""
        self.seq_generator = SeqGenerator(start=1)
        self.event_queue = EventQueue(maxsize=1000)
        self.initialized = False
        self.configurating_done = False
        self.running = False
    
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
