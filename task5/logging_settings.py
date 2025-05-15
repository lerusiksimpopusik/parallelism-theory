import logging
import os
from logging.handlers import RotatingFileHandler

if not os.path.exists('log'):
    os.makedirs('log')
try:
    os.remove(os.path.join('log', 'app.log'))
except FileNotFoundError:
    pass

logging.basicConfig(
    level=logging.INFO,
    handlers=[
        RotatingFileHandler('log/app.log', maxBytes=1024 * 1024, backupCount=5)
    ]
)
logger = logging.getLogger(__name__)