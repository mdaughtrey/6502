# Logging Configuration

This Debug Adapter Protocol implementation includes comprehensive logging to help with debugging and monitoring.

## Logging Features

- **Console Output**: All log messages are displayed in real-time on the console (stdout)
- **File Output**: All log messages are also written to `logs/debug_adapter.log`
- **Log Level**: DEBUG level is used for detailed handler entry/exit logging
- **Format**: Each log entry includes timestamp, logger name, log level, and message

## Log Levels Used

- **DEBUG**: Handler entry/exit with request and response details
- **INFO**: Server startup, client connections, configuration
- **ERROR**: Exceptions and errors during request handling

## Example Log Output

```
2026-02-16 23:13:17 - dap - INFO - Starting Debug Adapter on 0.0.0.0:8080
2026-02-16 23:13:17 - dap - DEBUG - Serial port configured: /dev/ttyUSB0
2026-02-16 23:13:17 - dap - INFO - Debug Adapter listening on 0.0.0.0:8080
2026-02-16 23:13:17 - dap - INFO - Client connected from ('127.0.0.1', 54321)
2026-02-16 23:13:17 - dap - DEBUG - Received request: {'command': 'initialize', 'seq': 1, ...}
2026-02-16 23:13:17 - dap - DEBUG - Handler InitializeHandler processing command 'initialize' (seq=1)
2026-02-16 23:13:17 - dap - DEBUG - Handler InitializeHandler completed command 'initialize' (seq=1) with success=True
2026-02-16 23:13:17 - dap - DEBUG - Sending response: {'type': 'response', 'seq': 1, ...}
```

## How Handler Logging Works

Each handler inherits from `BaseHandler`, which provides:

1. **Automatic Entry/Exit Logging**: The `_handle_with_logging()` method wraps the actual `handle()` method
2. **Per-Handler Logging**: Each handler logs its entry, completion status, and any errors
3. **Logger Instance**: Each handler has access to the module-level logger via `self.logger`

Example handler invocation flow:
```
Dispatcher.dispatch(request)
  ↓
Handler._handle_with_logging(request)  ← Logs request entry
  ↓
Handler.handle(request)                 ← Handler-specific logic
  ↓
Logger.debug("Handler X completed...")  ← Logs completion with success status
```

## Customizing Logging

To add custom logging in a handler:

```python
class CustomHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> Dict[str, Any]:
        # Log custom messages
        self.logger.debug(f"Custom processing for request: {request}")
        # ... handler logic ...
        return response
```

## Log File Location

- **Path**: `logs/debug_adapter.log`
- **Directory**: Created automatically if it doesn't exist
- **Permissions**: Readable and writable by all users in the group

## Clearing Logs

To clear the log file:

```bash
> rm logs/debug_adapter.log
```

The next run will create a fresh log file. Logs are appended, not truncated.
