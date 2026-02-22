"""Sequence number generator for DAP messages."""

import threading


class SeqGenerator:
    """Thread-safe sequence number generator for DAP messages."""
    
    def __init__(self, start: int = 1):
        """Initialize the generator.
        
        Args:
            start: Starting sequence number (default 1 per DAP spec)
        """
        self._seq = start
        self._lock = threading.Lock()
    
    def next(self) -> int:
        """Get the next sequence number."""
        with self._lock:
            result = self._seq
            self._seq += 1
            return result
    
    def current(self) -> int:
        """Get the current sequence number without incrementing."""
        with self._lock:
            return self._seq
