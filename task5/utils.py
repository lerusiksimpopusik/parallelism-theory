import os

import cv2
from pathlib import Path


from logging_settings import logger


def is_video_file(source):
    """
    Check, the existence of a file and video extension
    """
    return (os.path.isfile(source) and
            source.lower().endswith(('.mp4', '.avi', '.mov', '.mkv', '.wmv', '.flv')))


def get_video_resolution(input_path: str) -> tuple[int, int]:
    """
    Get the resolution (width, height) of a video file.
    Args:
        input_path: Path to the video file.
    Returns:
        tuple[int, int]: Width and height of the video.
    """
    try:
        cap = cv2.VideoCapture(input_path)
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

        if not cap.isOpened():
            raise IOError(f"Couldn't open source {input_path}")

        cap.release()

        return width, height

    except IOError as e:
        logger.error(e)
        raise e

    except Exception as e:
        logger.error(f"Source setup error : {input_path}, error: {e}")
        raise f"Source setup error : {input_path}, error: {e}"


def resize_video(input_path: str, resolution: tuple[int, int] = (640, 480)) -> None:
    """
    Resize the video file according to the given resolution.
    """
    temp_path = input_path.replace(Path(input_path).suffix, f"temp_{Path(input_path).suffix}")
    try:
        cap = cv2.VideoCapture(input_path)
        fps = cap.get(cv2.CAP_PROP_FPS)
        out = cv2.VideoWriter(temp_path, cv2.VideoWriter_fourcc(*'mp4v'), fps, resolution)

        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
            resized_frame = cv2.resize(frame, resolution)
            out.write(resized_frame)
        cap.release()
        out.release()

        os.remove(input_path)
        os.rename(temp_path, input_path)

    except IOError as e:
        logger.error(e)
        raise e

    except Exception as e:
        logger.error(f"Source setup error : {input_path}, error: {e}")
        raise f"Source setup error : {input_path}, error: {e}"