"""Background serial reader that forwards raw target output as DAP events."""

from __future__ import annotations

import json
import re
import threading

from typing import Callable, Optional, Any


class SerialListener:
    """Read bytes from a serial connection in a background thread.

    The listener intentionally does not assume a target wire protocol yet.
    Any received bytes are forwarded as generic DAP output events.
    """

    def __init__(
        self,
        serial_conn: Any,
        queue_event: Callable[[dict], None],
        logger,
    ) -> None:
        self.serial_conn = serial_conn
        self.queue_event = queue_event
        self.logger = logger
        self._stop_event = threading.Event()
        self._thread: Optional[threading.Thread] = None

    @property
    def is_running(self) -> bool:
        """Return True when the listener thread is active."""
        return self._thread is not None and self._thread.is_alive()

    def start(self) -> bool:
        """Start the listener thread; return True only on a fresh start."""
        if self.is_running:
            return False
        if self.serial_conn is None:
            self.logger.debug("SerialListener start requested without serial connection")
            return False

        # Force blocking reads for the listener thread.
        try:
            self.serial_conn.timeout = None
        except Exception:
            self.logger.debug("Unable to set serial timeout=None; using existing serial settings")

        self._stop_event.clear()
        self._thread = threading.Thread(
            target=self._run,
            name="SerialListener",
            daemon=True,
        )
        self._thread.start()
        self.logger.debug("Serial listener started")
        return True

    def stop(self, timeout: float = 1.0) -> bool:
        """Stop the listener thread; return True when a running thread was stopped."""
        if not self.is_running:
            return False

        self._stop_event.set()

        # If a blocking read is in progress, ask pyserial to cancel it.
        cancel_read = getattr(self.serial_conn, "cancel_read", None)
        if callable(cancel_read):
            try:
                cancel_read()
            except Exception:
                self.logger.debug("cancel_read() failed while stopping serial listener", exc_info=True)

        assert self._thread is not None
        self._thread.join(timeout=timeout)
        stopped = not self._thread.is_alive()
        if stopped:
            self.logger.debug("Serial listener stopped")
        else:
            self.logger.warning("Serial listener did not stop before timeout")
        return True

    def _run(self) -> None:
        accumulated = ""
        while not self._stop_event.is_set():
            try:
                # Blocking read: wait for a full line or cancel_read()/port close.
                raw = self.serial_conn.read_until(b"\n")
                raw = raw.strip(b"\r\n")
            except Exception as exc:
                # self._emit_diagnostic(f"Serial read error: {exc}")
                self.logger.exception(f"Serial listener read failed: {exc}")
                break

            if not raw:
                continue

            accumulated += raw.decode("utf-8", errors="replace")

            try:
                first = accumulated.find("{")
                last = accumulated.rfind("}")
                if first == -1 or last == -1 or last < first:
                    continue
                self.logger.debug(f"Serial received %s\r\n", accumulated[first:last+1])
                event = json.loads(accumulated[first:last+1])
                self.logger.debug(f"Serial listener received event: {event}")
                accumulated = ""
            except Exception:
                continue

            self.queue_event(event)

        self.logger.debug("Serial listener thread exiting")



    