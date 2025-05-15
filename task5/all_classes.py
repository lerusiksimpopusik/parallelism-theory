import multiprocessing as mp
import queue
import threading
import time
from queue import Queue
from typing import Union

import cv2
import numpy as np
from ultralytics import YOLO

from logging_settings import logger


class VideoProccessor:
    def __init__(self, input_path: str, output_path: str, is_video: bool):
        self.input_path = input_path
        self.is_video = is_video
        try:
            self.cap = cv2.VideoCapture(input_path)

            if is_video:
                self.fps = self.cap.get(cv2.CAP_PROP_FPS)
                self.frame_count = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
                self.out = cv2.VideoWriter(
                    output_path,
                    cv2.VideoWriter_fourcc(*'mp4v'),
                    self.fps,
                    (640, 480)
                )
            else:
                self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

            if not self.cap.isOpened():
                raise IOError(f"Couldn't open source {input_path}")

        except IOError as e:
            logger.error(e)
            raise e
        except Exception as e:
            logger.error(f"Source setup error: {input_path}, error: {e}")
            raise f"Source setup error: {input_path}, error: {e}"

    def __del__(self):
        if hasattr(self, 'cap') and self.cap.isOpened():
            self.cap.release()
        if hasattr(self, 'out') and self.out is not None and self.out.isOpened():
            self.out.release()

    def process_frame(self, model, frame: np.ndarray) -> np.ndarray:
        """Processing of one frame"""
        results = model(frame, verbose=False)
        return results[0].plot()


class SingleVideoProccessor(VideoProccessor):
    def real_time_process(self, model):
        cv2.namedWindow("RealTime YOLO-pose", cv2.WINDOW_NORMAL)
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    logger.error("Failed to get frame")
                    break

                processed_frame = self.process_frame(model, frame)
                cv2.imshow("RealTime YOLO-pose", processed_frame)

                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
        except Exception as e:
            logger.critical(f"Fatal error: {str(e)}", exc_info=True)
        finally:
            cv2.destroyAllWindows()

    def record_process(self, model) -> float:
        start_time = time.time()
        while True:
            ret, frame = self.cap.read()
            if not ret:
                break
            processed_frame = self.process_frame(model, frame)
            self.out.write(processed_frame)
        return time.time() - start_time

    def run(self) -> float | None:
        model = YOLO('yolov8s-pose.pt').to('cpu')
        if self.is_video:
            return self.record_process(model)
        else:
            self.real_time_process(model)
            return None


def worker(in_queue, out_queue, stop_event):
    local_model = YOLO('yolov8s-pose.pt').to('cpu')
    while not stop_event.is_set():
        try:
            frame_idx, frame = in_queue.get(timeout=0.1)
        except queue.Empty:
            continue

        if frame is None:
            break

        res = local_model(frame, verbose=False)
        out_queue.put((frame_idx, res[0].plot()))


def put_frames_to_queue(input_path: str, in_queue: Union[Queue, mp.Queue],
                        num_workers: int, is_camera: bool, stop_event):
    try:
        cap = cv2.VideoCapture(input_path)
        if is_camera:
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

        frame_idx = 0
        while not stop_event.is_set():
            ret, frame = cap.read()
            if not ret:
                break

            in_queue.put((frame_idx, frame))
            frame_idx += 1

            if is_camera and cv2.waitKey(1) & 0xFF == ord('q'):
                stop_event.set()
                break
    finally:
        cap.release()


class MultiVideoProccessor(VideoProccessor):
    def __init__(self, input_path: str, output_path: str, num_workers: int, parallel_type: str, is_video: bool):
        super().__init__(input_path, output_path, is_video)
        self.num_workers = num_workers
        self.parallel_type = parallel_type
        if hasattr(self, 'cap'):
            self.cap.release()

    def run(self) -> float:

        if not self.is_video:
            cv2.namedWindow("MultiProcess YOLO-pose", cv2.WINDOW_NORMAL)

        if self.parallel_type == "thread":
            in_queue = Queue()
            out_queue = Queue()
            worker_class = threading.Thread
            stop_event = threading.Event()
        else:
            mp.set_start_method('spawn', force=True)
            in_queue = mp.Queue()
            out_queue = mp.Queue()
            worker_class = mp.Process
            stop_event = mp.Event()

        workers = []
        for _ in range(self.num_workers):
            worker_obj = worker_class(target=worker, args=(in_queue, out_queue, stop_event))
            worker_obj.start()
            workers.append(worker_obj)

        producer_obj = worker_class(
            target=put_frames_to_queue,
            args=(self.input_path, in_queue, self.num_workers, not self.is_video, stop_event)
        )
        producer_obj.start()

        frame_buffer = {}
        next_frame_idx = 0
        start_time = time.time()
        processed_count = 0

        try:
            while not stop_event.is_set():
                if self.is_video and processed_count >= self.frame_count:
                    stop_event.set()
                    break

                try:
                    frame_idx, processed_frame = out_queue.get(timeout=0.1)
                except queue.Empty:
                    continue

                frame_buffer[frame_idx] = processed_frame

                while next_frame_idx in frame_buffer:
                    if self.is_video:
                        self.out.write(frame_buffer.pop(next_frame_idx))
                    else:
                        cv2.imshow("MultiProcess YOLO-pose", frame_buffer.pop(next_frame_idx))
                        if cv2.waitKey(1) & 0xFF == ord('q'):
                            stop_event.set()
                            break

                    next_frame_idx += 1
                    processed_count += 1

        except KeyboardInterrupt:
            logger.info("Processing interrupted by user")
            stop_event.set()
        finally:
            stop_event.set()
            producer_obj.join()
            for worker_obj in workers:
                worker_obj.join()

            if not self.is_video:
                cv2.destroyAllWindows()

        return time.time() - start_time