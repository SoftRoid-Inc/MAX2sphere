import argparse
import os
import cv2
from datetime import datetime as dt
import subprocess
from glob import glob
from tqdm import tqdm
from multiprocessing import Process
from pathlib import Path
# from .base_process import BaseProcess
# from repositories import PolishS3Repository
# from config import POLISH_BUCKET_NAME
import datetime

# lib paths
MAX2SPHERE_PATH = "/root/vslam/submodules/MAX2sphere"

class MAX2sphereC(object):
    input_track0_dirname = "testframes/track0"
    input_track5_dirname = "testframes/track5"
    output_image_dirname = "images/er"
    # s3_repository = PolishS3Repository(POLISH_BUCKET_NAME)

    def __init__(self):
        # content path
        # self.parse_raw_video_s3_path(s3_path, pipeline_start_datetime=pipeline_start_datetime, prev_start_datetime=prev_restart_datetime)
        # self.thread_ts = thread_ts

        self.input_track0_dir = f"./{self.input_track0_dirname}"
        self.input_track5_dir = f"./{self.input_track5_dirname}"
        self.output_image_dir = f"./{self.output_image_dirname}"

    def download_for_restart(self):
        prefix = f"{self.prev_s3_base_dir}/{self.input_track0_dirname}"
        dest_dir_path = self.input_track0_dir
        self.s3_repository.fetch_by_prefix(prefix=prefix, dest_dir_path=dest_dir_path)
        prefix = f"{self.prev_s3_base_dir}/{self.input_track5_dirname}"
        dest_dir_path = self.input_track5_dir
        self.s3_repository.fetch_by_prefix(prefix=prefix, dest_dir_path=dest_dir_path)


    def _check_result(self):
        output_fpaths = glob(f"{self.output_image_dir}/*.jpg")
        self.check_inout_num_match(input_file_num=min(len(self.input_track0_filepaths),len(self.input_track5_filepaths)),
         output_file_num=len(output_fpaths), debug=True, thread_ts=self.thread_ts)


    def __call__(self):
        self.input_track0_filepaths = glob(f"{self.input_track0_dir}/*.jpg")
        self.input_track5_filepaths = glob(f"{self.input_track5_dir}/*.jpg")
        self.input_track0_filepaths.sort()
        self.input_track5_filepaths.sort()

        os.makedirs(self.output_image_dir, exist_ok=True)
        def max2sphere(track0_image_path: str, track5_image_path: str):
            track0_image_fname = os.path.basename(track0_image_path)
            track5_image_fname = os.path.basename(track5_image_path)
            assert track0_image_fname == track5_image_fname, "track0 and track5 image filenames are not same"
            track0_fname_wo_ext = Path(track0_image_fname).stem
            idx = track0_fname_wo_ext.split("_")[-1]

            output_filepath = f"{self.output_image_dir}/er_frmame_{idx}.jpg"
            # command = f"./MAX2sphere -o {output_filepath} {track0_image_path} {track5_image_path}"# https://github.com/SoftRoid-Inc/MAX2sphere/blob/main/MAX2sphere.c#L562
            command = f"./MAX2sphere -r './precalc.bin' -o {output_filepath} {track0_image_path} {track5_image_path}"# https://github.com/SoftRoid-Inc/MAX2sphere/blob/main/MAX2sphere.c#L562

            # command = f"{MAX2SPHERE_PATH}/MAX2sphere -o {output_filepath} {track0_image_path} {track5_image_path}"# https://github.com/SoftRoid-Inc/MAX2sphere/blob/main/MAX2sphere.c#L562
            # command = f"{MAX2SPHERE_PATH}/MAX2sphere {track0_image_path} {track5_image_path}" # https://github.com/SoftRoid-Inc/MAX2sphere/blob/main/MAX2sphere.c#L562
            subprocess.run(command, shell=True)

        processes = []
        num_parallel = os.cpu_count()-2
        for track0_image_path, track5_image_path in tqdm(zip(self.input_track0_filepaths, self.input_track5_filepaths)):
            while len(processes) > num_parallel:
                processes[0].join()
                del processes[0]
            processes.append(Process(target=max2sphere, args=(track0_image_path, track5_image_path)))
            processes[-1].start()
        for process in processes:
            process.join()
        
        # self._check_result()


if __name__ == '__main__':
    # parser = argparse.ArgumentParser()
    # parser.add_argument("--input_video_path", type=str)
    # args = parser.parse_args()
    gopro_track_extractor =MAX2sphereC()
    gopro_track_extractor()

