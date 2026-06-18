import time

from src.serial_listener import SerialListener


class FakeSerial:
    def __init__(self, chunks=None, fail_after_reads=None):
        self._chunks = list(chunks or [])
        self._read_calls = 0
        self._fail_after_reads = fail_after_reads

    @property
    def in_waiting(self):
        if not self._chunks:
            return 0
        return len(self._chunks[0])

    def read(self, size=1):
        self._read_calls += 1
        if self._fail_after_reads is not None and self._read_calls > self._fail_after_reads:
            raise OSError("simulated serial failure")

        if self._chunks:
            chunk = self._chunks.pop(0)
            if len(chunk) > size:
                self._chunks.insert(0, chunk[size:])
                return chunk[:size]
            return chunk

        time.sleep(0.01)
        return b""


class DummyLogger:
    def debug(self, *args, **kwargs):
        return None

    def info(self, *args, **kwargs):
        return None

    def warning(self, *args, **kwargs):
        return None

    def exception(self, *args, **kwargs):
        return None


def _wait_for(predicate, timeout=1.0):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return True
        time.sleep(0.01)
    return False


def test_serial_listener_start_stop_idempotent():
    events = []
    listener = SerialListener(
        serial_conn=FakeSerial(),
        queue_event=events.append,
        logger=DummyLogger(),
    )

    assert listener.start() is True
    assert listener.start() is False
    assert listener.stop() is True
    assert listener.stop() is False


def test_serial_listener_queues_output_events():
    events = []
    listener = SerialListener(
        serial_conn=FakeSerial(chunks=[b"hello", b" world"]),
        queue_event=events.append,
        logger=DummyLogger(),
    )

    assert listener.start() is True
    assert _wait_for(lambda: len(events) >= 2)
    assert listener.stop() is True

    outputs = [evt["body"]["output"] for evt in events if evt.get("event") == "output"]
    assert outputs
    assert "hello" in "".join(outputs)


def test_serial_listener_emits_diagnostic_on_read_error():
    events = []
    listener = SerialListener(
        serial_conn=FakeSerial(chunks=[b"ok"], fail_after_reads=1),
        queue_event=events.append,
        logger=DummyLogger(),
    )

    assert listener.start() is True
    assert _wait_for(
        lambda: any(
            evt.get("body", {}).get("category") == "stderr"
            for evt in events
            if evt.get("event") == "output"
        )
    )
    assert listener.stop() in {True, False}
