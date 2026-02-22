import json
from src.handlers.dispatcher import Dispatcher


def test_initialize_response():
    dispatcher = Dispatcher(serial_port=None)
    request = {
        "seq": 1,
        "type": "request",
        "command": "initialize",
        "arguments": {"adapterID": "test", "pathFormat": "path"},
    }

    response, events = dispatcher.dispatch(request)
    assert isinstance(response, dict)
    assert response.get("type") == "response"
    assert response.get("request_seq") == 1
    assert response.get("command") == "initialize"
    assert response.get("success") is True
    
    # Check that initialize event is returned
    assert isinstance(events, list)
    assert len(events) == 1
    assert events[0].get("type") == "event"
    assert events[0].get("event") == "initialized"
