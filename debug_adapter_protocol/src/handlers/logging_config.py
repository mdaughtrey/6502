"""Logging configuration for DAP handlers."""

import logging
from pathlib import Path


def setup_logging(log_file: str = "debug_adapter.log", console_level: str = "INFO") -> logging.Logger:
    """Set up logging to both console and file.
    
    Args:
        log_file: Path to the log file.
        console_level: Log level for console handler.
        
    Returns:
        A configured logger instance.
    """
    # Create logs directory if it doesn't exist
    log_dir = Path("logs")
    log_dir.mkdir(exist_ok=True)
    
    log_path = log_dir / log_file
    
    # Create logger
    logger = logging.getLogger("dap")
    logger.setLevel(logging.DEBUG)

    level_map = {
        "CRITICAL": logging.CRITICAL,
        "ERROR": logging.ERROR,
        "WARNING": logging.WARNING,
        "INFO": logging.INFO,
        "DEBUG": logging.DEBUG,
        "NOTSET": logging.NOTSET,
    }
    normalized_console_level = str(console_level).upper()
    if normalized_console_level not in level_map:
        allowed = ", ".join(level_map.keys())
        raise ValueError(f"Invalid console log level '{console_level}'. Allowed values: {allowed}")
    
    # Clear any existing handlers
    logger.handlers.clear()
    
    # Create formatter
    formatter = logging.Formatter(
        fmt="%(asctime)s - %(name)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    
    # Console handler
    console_handler = logging.StreamHandler()
    console_handler.setLevel(level_map[normalized_console_level])
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)
    
    # File handler
    file_handler = logging.FileHandler(log_path, mode='a')
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    
    return logger


# Get or create the logger
logger = logging.getLogger("dap")
if not logger.handlers:
    setup_logging()
