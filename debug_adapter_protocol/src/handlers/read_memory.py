"""Handler for readMemory requests."""
import base64
from typing import Any, Dict
from .base_handler import BaseHandler

class ReadMemoryHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        try:
            args = request.get('arguments')
            address = int(args.get('memoryReference'), 16)
            length = args.get('count')
            response = self.backend_session.read_memory(address, length)
            response["data"] = base64.b64encode(bytes([int(xx, 16) for xx in response.get("data")])).decode("ascii")
            return self.create_response(request, body=response), []
        except Exception as e:
            self.logger.error(f"Error in read memory handler: {str(e)}", exc_info=True)
            return self.create_response(request, success=False, message=f"Error reading memory: {str(e)}"), []
        return self.create_response(request, body={"address": "0x0", "data": ""}), []
