import os
import sys

# Ensure the repository root is on sys.path so `src` can be imported as a
# namespace package (PEP 420).
ROOT = os.path.dirname(os.path.dirname(__file__))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)
