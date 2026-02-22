"""Thread-safe event queue for DAP events."""

import queue
import threading


class EventQueue:
    """Thread-safe FIFO queue for DAP events.
    
    Events can be added from request handlers or background tasks,
    and retrieved by the adapter for sending to clients.
    """
    
    def __init__(self, maxsize: int = 100):
        """Initialize the event queue.
        
        Args:
            maxsize: Maximum queue size (0 = unlimited)
        """
        self._queue: queue.Queue = queue.Queue(maxsize=maxsize)
    
    def put(self, event: dict) -> None:
        """Add an event to the queue (blocking if full).
        
        Args:
            event: DAP event dict with type='event'
        """
        self._queue.put(event)
    
    def put_nowait(self, event: dict) -> None:
        """Add an event without blocking; raises if full.
        
        Args:
            event: DAP event dict with type='event'
            
        Raises:
            queue.Full: If queue is at max capacity
        """
        self._queue.put_nowait(event)
    
    def get(self, timeout: float = None):
        """Get an event (blocking).
        
        Args:
            timeout: Timeout in seconds; None = block forever
            
        Returns:
            DAP event dict or None if timeout (with timeout=0)
            
        Raises:
            queue.Empty: If timeout expires
        """
        try:
            return self._queue.get(timeout=timeout)
        except queue.Empty:
            raise
    
    def get_nowait(self):
        """Get an event without blocking.
        
        Returns:
            DAP event dict
            
        Raises:
            queue.Empty: If queue is empty
        """
        return self._queue.get_nowait()
    
    def get_all(self) -> list:
        """Get all pending events (non-blocking).
        
        Returns:
            List of all pending events; empty list if none
        """
        events = []
        while True:
            try:
                events.append(self._queue.get_nowait())
            except queue.Empty:
                break
        return events
    
    def empty(self) -> bool:
        """Check if queue is empty."""
        return self._queue.empty()
    
    def qsize(self) -> int:
        """Get approximate queue size."""
        return self._queue.qsize()
