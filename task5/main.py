import os

import click

from all_classes import SingleVideoProccessor, MultiVideoProccessor
from logging_settings import logger
from utils import get_video_resolution, resize_video, is_video_file


@click.command()
@click.argument("input_path", type=str)
@click.option(
    "--regime", "-r",
    type=click.Choice(["thread", "process"], case_sensitive=False),
    default="process",
    show_default=True,
    help="Parallelization: 'Thread' or 'Process'."
)
@click.option(
    "--num_workers", "-n",
    type=int,
    default=1,
    show_default=True,
    help="The number of flows/processes."
)
def main(input_path: str, regime: str, num_workers: int) :

    if is_video_file(input_path):
        height, width = get_video_resolution(input_path)

        if (height, width) != (640, 480):
            print(f"Change in resolution with {width} x {height} by 640x480 ...")
            resize_video(input_path, (640, 480))

    try:
        if num_workers == 1:
            processor = SingleVideoProccessor(input_path, f"result_{os.path.basename(input_path)}", is_video_file(input_path))
        else:
            processor = MultiVideoProccessor(input_path, f"result_{os.path.basename(input_path)}", num_workers, regime, is_video_file(input_path))

        time_elapsed = processor.run()
        if time_elapsed is not None:
            print(f"Обработка заняла {time_elapsed:.2f} сек")

    except Exception as e:
        logger.error(e)
        raise e


if __name__ == '__main__':
    main()