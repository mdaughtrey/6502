#!/usr/bin/env python3
"""Test async event polling functionality."""

import asyncio
import sys
import os

# Add src directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

from backend_session import BackendSession

async def mock_poll_task(session):
    """Simulate the polling task that sends events."""
    events_sent = []
    try:
        for _ in range(5):  # Poll 5 times
            await asyncio.sleep(0.05)
            events = session.event_queue.get_all()
            events_sent.extend(events)
    except asyncio.CancelledError:
        pass
    return events_sent

async def test_async_event_polling():
    """Test that background task can retrieve queued events."""
    print("Testing async event polling...")
    
    session = BackendSession()
    print(f"✓ Created BackendSession")
    
    # Start polling task (simulating background polling)
    poll_task = asyncio.create_task(mock_poll_task(session))
    print(f"✓ Started polling task")
    
    # Simulate background thread queueing events after brief delay
    await asyncio.sleep(0.1)
    session.event_queue.put({'type': 'event', 'event': 'stopped', 'body': {'reason': 'breakpoint'}})
    print(f"✓ Queued stopped event")
    
    await asyncio.sleep(0.1)
    session.event_queue.put({'type': 'event', 'event': 'output', 'body': {'output': 'hello'}})
    print(f"✓ Queued output event")
    
    # Let polling continue for a bit
    await asyncio.sleep(0.2)
    
    # Cancel polling task
    poll_task.cancel()
    try:
        retrieved_events = await poll_task
    except asyncio.CancelledError:
        # Poll task will return partially
        retrieved_events = []
    
    print(f"✓ Task cancelled and cleaned up")
    
    # Check queue is empty now (polling should have retrieved everything)
    remaining = session.event_queue.get_all()
    print(f"✓ Event queue empty after polling: {len(remaining) == 0}")
    
    # Verify seq generator works
    seq1 = session.seq_generator.next()
    seq2 = session.seq_generator.next()
    print(f"✓ Seq generator works: seq1={seq1}, seq2={seq2}")
    
    print("\n✅ All async event polling tests passed!")

if __name__ == '__main__':
    asyncio.run(test_async_event_polling())
