import os
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
HELPDESK_EXE = os.path.join(BASE_DIR, "helpdesk")
if sys.platform.startswith("win"):
    HELPDESK_EXE += ".exe"

MAX_IMAGE_DIMENSION = 1024
JPEG_QUALITY = 85
