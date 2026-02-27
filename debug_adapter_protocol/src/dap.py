import typer
import json
import sys
import logging
import asyncio
from typing import Any, Dict
import socket
from handlers.dispatcher import Dispatcher
from handlers.logging_config import setup_logging, logger
from backend_session import BackendSession

app = typer.Typer()


def send_framed_message(writer: Any, msg: Dict[str, Any], logger_inst: Any = None) -> None:
    """Send a framed DAP message (response or event) over output.
    
    Args:
        writer: Output writer (file-like or asyncio writer)
        msg: Message dict (response or event)
        logger_inst: Optional logger instance
    """
    payload = json.dumps(msg).encode('utf-8')
    header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
    if hasattr(writer, 'write'):  # Sync writer
        writer.write(header_bytes + payload)
    else:  # AsyncIO writer
        writer.write(header_bytes + payload)


@app.callback(invoke_without_command=True)
def main(
    port: int = typer.Option(8080, help="The port to run on"),
    host: str = typer.Option("0.0.0.0", help="The host to run on"),
    serial_port: str = typer.Option(
        "/dev/tty.usbmodem212101",
        "--serial-port",
        "--serial",
        help="The serial port to use",
    ),
    protocol: str = typer.Option("tcp", help="Protocol to use: 'tcp' or 'ws' (websocket)")
):
    """Start the Debug Adapter Protocol listener. This runs when the script is executed
    without a subcommand so `python src/dap.py --protocol tcp` works directly.
    """
    # Initialize logging
    setup_logging()
    logger.info(f"Starting Debug Adapter on {host}:{port}")
    logger.debug(f"Serial port configured: {serial_port}")

    if protocol.lower() in ("ws", "websocket", "websockets"):
        # Start websocket server (async)
        try:
            asyncio.run(run_websocket_server(host, port, serial_port))
        except Exception as e:
            logger.error(f"WebSocket server failed to start: {e}", exc_info=True)
    else:
        # Run the async TCP server for better concurrency
        try:
            asyncio.run(run_tcp_server(host, port, serial_port))
        except Exception as e:
            logger.error(f"TCP server failed to start: {e}", exc_info=True)


@app.command("run")
def run(
    port: int = typer.Option(8080, help="The port to run on"),
    host: str = typer.Option("0.0.0.0", help="The host to run on"),
    serial_port: str = typer.Option(
        "/dev/tty.usbmodem212101",
        "--serial-port",
        "--serial",
        help="The serial port to use",
    ),
    protocol: str = typer.Option("tcp", help="Protocol to use: 'tcp' or 'ws' (websocket)")
):
    """Alias for the main command for compatibility with `run` subcommand usage."""
    return main(port=port, host=host, serial_port=serial_port, protocol=protocol)


def listen_for_requests(host: str, port: int, dispatcher: Dispatcher):
    """Listen for incoming DAP requests on the specified host and port."""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((host, port))
    server.listen(1)
    
    logger.info(f"Debug Adapter listening on {host}:{port}")
    print(f"Debug Adapter listening on {host}:{port}")
    
    try:
        while True:
            client, addr = server.accept()
            logger.info(f"Client connected from {addr}")
            handle_client(client, dispatcher)
    except KeyboardInterrupt:
        logger.info("Shutting down Debug Adapter")
    finally:
        server.close()


async def poll_tcp_events(backend_session: BackendSession, writer: asyncio.StreamWriter, peer):
    """Async task that polls event queue and sends spontaneous events.
    
    Runs in parallel with request handling to send events as soon as they're queued.
    """
    try:
        while True:
            await asyncio.sleep(0.05)  # Poll every 50ms for responsive events
            
            # Get all pending events from queue (non-blocking)
            events = backend_session.event_queue.get_all()
            
            for event in events:
                if isinstance(event, dict):
                    try:
                        event['seq'] = backend_session.seq_generator.next()
                        payload = json.dumps(event).encode('utf-8')
                        header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
                        writer.write(header_bytes + payload)
                        await writer.drain()
                        logger.debug(f"Sent spontaneous TCP event: {event.get('event', 'unknown')}")
                    except Exception:
                        logger.exception(f'Failed to send spontaneous TCP event to {peer}')
    except asyncio.CancelledError:
        logger.debug(f"Event polling stopped for {peer}")
    except Exception as e:
        logger.error(f"TCP event polling error for {peer}: {e}", exc_info=True)


async def run_tcp_server(host: str, port: int, serial: str):
    """Async TCP server that speaks DAP framing (Content-Length).

    Uses asyncio streams so the server can handle multiple clients concurrently.
    """
    async def handle(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        peer = writer.get_extra_info('peername')
        logger.info(f"TCP client connected: {peer}")
        
        # Create per-connection session and dispatcher
        backend_session = BackendSession()
        dispatcher = Dispatcher(serial_port=serial, backend_session=backend_session)
        
        # Start background task for spontaneous event polling
        poll_task = asyncio.create_task(poll_tcp_events(backend_session, writer, peer))
        
        buffer = b""
        try:
            while True:
                data = await reader.read(4096)
                if not data:
                    logger.debug(f"TCP client disconnected: {peer}")
                    break
                buffer += data

                while True:
                    header_end = buffer.find(b"\r\n\r\n")
                    if header_end == -1:
                        break

                    header = buffer[:header_end].decode('ascii', errors='ignore')
                    content_length = None
                    for line in header.split('\r\n'):
                        if line.lower().startswith('content-length:'):
                            try:
                                content_length = int(line.split(':', 1)[1].strip())
                            except Exception:
                                content_length = None
                    if content_length is None:
                        logger.error('Missing Content-Length header; closing connection')
                        writer.close()
                        await writer.wait_closed()
                        return

                    message_start = header_end + 4
                    message_end = message_start + content_length
                    if len(buffer) < message_end:
                        break

                    content_bytes = buffer[message_start:message_end]
                    buffer = buffer[message_end:]

                    try:
                        message_text = content_bytes.decode('utf-8')
                        request = json.loads(message_text)
                    except json.JSONDecodeError:
                        logger.error('Invalid JSON payload received')
                        continue

                    logger.debug(f"Received request: {request}")
                    # dispatch synchronously; handlers may be sync for now
                    response, events = dispatcher.dispatch(request)

                    if response:
                        # Assign sequence number from backend session
                        response['seq'] = backend_session.seq_generator.next()
                        payload = json.dumps(response).encode('utf-8')
                        header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
                        try:
                            writer.write(header_bytes + payload)
                            await writer.drain()
                            logger.debug(f"Sent framed TCP response of {len(payload)} bytes to {peer}")
                        except Exception:
                            logger.exception('Failed to send response to TCP client')
                    
                    # Send any returned events from handler
                    for event in events:
                        if isinstance(event, dict):
                            event['seq'] = backend_session.seq_generator.next()
                            payload = json.dumps(event).encode('utf-8')
                            header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
                            try:
                                writer.write(header_bytes + payload)
                                await writer.drain()
                                logger.debug(f"Sent framed TCP event: {event.get('event', 'unknown')}")
                            except Exception:
                                logger.exception('Failed to send event to TCP client')
        except Exception as e:
            logger.error(f"TCP handler error for {peer}: {e}", exc_info=True)
        finally:
            # Cancel the background polling task
            poll_task.cancel()
            try:
                await poll_task
            except asyncio.CancelledError:
                pass
            try:
                writer.close()
                await writer.wait_closed()
            except Exception:
                pass

    server = await asyncio.start_server(handle, host, port)
    addr = server.sockets[0].getsockname()
    logger.info(f"Async TCP Debug Adapter listening on {addr}")
    async with server:
        await server.serve_forever()


def handle_client(client: socket.socket, dispatcher: Dispatcher):
    """Handle a single client connection."""
    buffer = b""
    try:
        while True:
            data = client.recv(4096)
            if not data:
                logger.debug("Client disconnected")
                break

            buffer += data

            # Parse zero or more complete DAP messages from buffer
            while True:
                header_end = buffer.find(b"\r\n\r\n")
                if header_end == -1:
                    # not enough data for headers
                    break

                header = buffer[:header_end].decode('ascii', errors='ignore')
                content_length = None
                for line in header.split('\r\n'):
                    if line.lower().startswith('content-length:'):
                        try:
                            content_length = int(line.split(':', 1)[1].strip())
                        except Exception:
                            content_length = None
                if content_length is None:
                    logger.error('Missing Content-Length header; closing connection')
                    return

                message_start = header_end + 4
                message_end = message_start + content_length

                if len(buffer) < message_end:
                    # wait for more of the body
                    break

                content_bytes = buffer[message_start:message_end]
                buffer = buffer[message_end:]

                try:
                    message_text = content_bytes.decode('utf-8')
                    request = json.loads(message_text)
                except json.JSONDecodeError:
                    logger.error('Invalid JSON payload received')
                    continue

                logger.debug(f"Received request: {request}")
                response, events = dispatcher.dispatch(request)

                if response:
                    payload = json.dumps(response).encode('utf-8')
                    header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
                    try:
                        client.sendall(header_bytes + payload)
                        logger.debug(f"Sent framed response of {len(payload)} bytes")
                    except Exception:
                        logger.error('Failed to send response to client', exc_info=True)
                
                # Send any events
                for event in events:
                    if isinstance(event, dict):
                        event['seq'] = seq_gen.next()
                        payload = json.dumps(event).encode('utf-8')
                        header_bytes = f"Content-Length: {len(payload)}\r\n\r\n".encode('ascii')
                        try:
                            client.sendall(header_bytes + payload)
                            logger.debug(f"Sent framed event: {event.get('event', 'unknown')}")
                        except Exception:
                            logger.error('Failed to send event to client', exc_info=True)
    except Exception as e:
        try:
            peer = client.getpeername()
        except Exception:
            peer = None
        logger.error(f"Error handling client {peer}: {str(e)}", exc_info=True)
    finally:
        client.close()

# Helper: attempt to extract JSON from either raw JSON or
# DAP-framed message that includes a Content-Length header.
def _parse_ws_message(text: str):
    stripped = text.lstrip()
    if stripped.startswith('{') or stripped.startswith('['):
        try:
            return json.loads(text)
        except json.JSONDecodeError:
            return None

    header_end = text.find('\r\n\r\n')
    if header_end != -1:
        header = text[:header_end]
        content_length = None
        for line in header.split('\r\n'):
            if line.lower().startswith('content-length:'):
                try:
                    content_length = int(line.split(':', 1)[1].strip())
                except Exception:
                    content_length = None
        if content_length is None:
            logger.error('Missing Content-Length header in ws message')
            return None

        body_start = header_end + 4
        body = text[body_start:body_start + content_length]
        try:
            return json.loads(body)
        except json.JSONDecodeError:
            logger.error('Invalid JSON body in ws framed message')
            return None

    # Fallback: try parsing the whole text
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return None

async def poll_websocket_events(backend_session: BackendSession, websocket, client):
    """Async task that polls event queue and sends spontaneous events over WebSocket.
    
    Runs in parallel with request handling to send events as soon as they're queued.
    """
    try:
        while True:
            await asyncio.sleep(0.05)  # Poll every 50ms for responsive events
            
            # Get all pending events from queue (non-blocking)
            events = backend_session.event_queue.get_all()
            
            for event in events:
                if isinstance(event, dict):
                    try:
                        event['seq'] = backend_session.seq_generator.next()
                        body = json.dumps(event)
                        event_msg = f"Content-Length: {len(body)}\r\n\r\n{body}"
                        await websocket.send(event_msg)
                        logger.debug(f"Sent spontaneous ws event: {event.get('event', 'unknown')}")
                    except Exception:
                        logger.exception(f'Failed to send spontaneous ws event to {client}')
    except asyncio.CancelledError:
        logger.debug(f"Event polling stopped for {client}")
    except Exception as e:
        logger.error(f"WebSocket event polling error for {client}: {e}", exc_info=True)


async def run_websocket_server(host: str, port: int, serial: str):
    """Run a WebSocket server that accepts JSON DAP messages.

    Tries to import the `websockets` package; if not available logs an error
    and returns so the TCP listener can be used instead.
    """
    try:
        import websockets
    except Exception:
        logger.error("websockets package not installed; cannot start ws server. Install with: poetry add websockets")
        return

    async def ws_handler(websocket, path=None):
        # Accept `path` optionally to support different `websockets` versions
        try:
            client = websocket.remote_address
        except Exception:
            client = None
        logger.info(f"WebSocket client connected: {client}")
        
        # Create per-connection session and dispatcher
        backend_session = BackendSession()
        dispatcher = Dispatcher(serial_port=serial, backend_session=backend_session)
        
        # Start background task for spontaneous event polling
        poll_task = asyncio.create_task(poll_websocket_events(backend_session, websocket, client))
        
        try:
            logger.debug("top of cycle")
            async for message in websocket:
                logger.debug("message received from ws client")
                # websockets may yield bytes or str depending on client; normalize
                if isinstance(message, bytes):
                    try:
                        message_text = message.decode('utf-8')
                    except Exception:
                        logger.error('Received non-UTF8 websocket message')
                        continue
                else:
                    message_text = message

                request = _parse_ws_message(message_text)
                if request is None:
                    logger.error('Invalid JSON received over websocket')
                    continue

                logger.debug(f"Received ws request: {request}")
                response, events = dispatcher.dispatch(request)
                if response:
                    try:
                        # Assign sequence number from backend session
                        response['seq'] = backend_session.seq_generator.next()
                        body = json.dumps(response)
                        response_msg = f"Content-Length: {len(body)}\r\n\r\n{body}"
                        await websocket.send(response_msg)
                        logger.debug(f"Sent ws response")
                    except Exception:
                        logger.exception('Failed to send ws response')
                
                # Send any returned events from handler
                for event in events:
                    if isinstance(event, dict):
                        event['seq'] = backend_session.seq_generator.next()
                        try:
                            body = json.dumps(event)
                            event_msg = f"Content-Length: {len(body)}\r\n\r\n{body}"
                            await websocket.send(event_msg)
                            logger.debug(f"Sent ws event: {event.get('event', 'unknown')}")
                        except Exception:
                            logger.exception('Failed to send ws event')

        except Exception as e:
            logger.exception(f"WebSocket handler error for {client}: {e}")
        finally:
            # Cancel the background polling task
            poll_task.cancel()
            try:
                await poll_task
            except asyncio.CancelledError:
                pass

    server = await websockets.serve(ws_handler, host, port)
    logger.info(f"WebSocket Debug Adapter listening on {host}:{port}")
    try:
        await asyncio.Future()  # run forever
    finally:
        server.close()
        await server.wait_closed()


def extract_message(buffer: str) -> tuple:
    """Legacy placeholder. DAP framing is now handled in `handle_client`.

    This function remains for backward compatibility but is not used.
    """
    return None, buffer


if __name__ == '__main__':
    app()
