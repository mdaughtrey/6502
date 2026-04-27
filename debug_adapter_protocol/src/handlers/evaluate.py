"""Handler for evaluate requests."""
from typing import Any, Dict
from .base_handler import BaseHandler


class EvaluateHandler(BaseHandler):
    def handle(self, request: Dict[str, Any]) -> tuple[Dict[str, Any], list]:
        # memory read 0x0400 256
        try:
            args = request.get('arguments').get("expression").split(' ')
            if args[0:2] == ['memory', 'read']:
                address = int(args[2], 16)
                length = int(args[3], 16)
                result = self.backend_session.read_memory(address, length)
                data = result.get("data")
                lines = len(data) // 16
                textdata = ""
                for ii in range(lines):
                    textdata += f"{address+(16*ii):04x} {' '.join(data[ii*16:(ii+1)*16])}\r\n"

                if len(data) % 16 != 0:
                    textdata += f"{address+(16*lines):04x} {' '.join(data[lines*16:(lines+1)*16])}\r\n"

                return self.create_response(request, body={"result": textdata,
                                                           "variablesReference": 0,
                                                           "memoryReference": f"0x{address:04x}",
                                                           "presentationHint": "data"}), []
        except Exception as e:
            self.logger.error(f"Error in evaluate handler: {str(e)}", exc_info=True)
            return self.create_response(request, success=False, message=f"Error evaluating expression: {str(e)}"), []
