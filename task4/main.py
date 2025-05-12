import click
import cv2
import time
import logging
from all_classes import *

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@click.command()
@click.option("--camera_name", "-c", default="0", type=str, help="Для Windows используйте 0")
@click.option("--resolution", "-r", default=(640, 480), type=(int, int), help="Разрешение камеры")
@click.option("--fps", "-f", default=30, type=int, help="Частота обновления кадров")
def main(camera_name, resolution, fps):
    camera = None
    sensors = []
    
    try:
        camera = SensorCam(camera_name, resolution)
        logger.info(f"Камера {camera_name} инициализирована")
        
        sensors = [
            SensorX(0.01),
            SensorX(0.1),
            SensorX(1.0)
        ]
        
        # Запуск потоков
        camera.start()
        for sensor in sensors:
            sensor.start()
            logger.info(f"Сенсор {sensor.name} запущен")

        window = WindowImage(fps)
        last_values = {}
        last_frames = []
        max_frames = 2

        while True:
            # Проверка ошибок камеры
            if camera.critical_error.is_set():
                logger.error("Ошибка камеры!")
                break

            # Получение кадра
            frame = camera.get()
            if frame is not None:
                last_frames.append(frame)
                if len(last_frames) > max_frames:
                    last_frames.pop(0)
            
            current_frame = last_frames[-1] if last_frames else None

            # Получение данных сенсоров
            sensor_values = {}
            for sensor in sensors:
                value = sensor.get()
                sensor_values[sensor.name] = last_values.get(sensor.name, 0) if value is None else value
                last_values[sensor.name] = sensor_values[sensor.name]

            # Отображение
            if current_frame is not None:
                window.show(current_frame, sensor_values)
            else:
                logger.warning("Нет кадров для отображения")

            # Выход по 'q'
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except Exception as e:
        logger.critical(f"Фатальная ошибка: {str(e)}", exc_info=True)
    finally:
        if camera is not None:
            camera.stop()
        for sensor in sensors:
            sensor.stop()
        cv2.destroyAllWindows()
        logger.info("Программа завершена")

if __name__ == '__main__':
    main()