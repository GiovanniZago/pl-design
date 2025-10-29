import config

import os
import subprocess

def get_xilinx_environment():
    command = f'bash -c "source {config.XRT_SETUP} && env"'
    result = subprocess.run(command, capture_output=True, text=True, shell=True)

    env = {}
    for line in result.stdout.splitlines():
        key, _, value = line.partition("=")
        env[key] = value

    return env

def hls_compile(env, kernel_name):
    os.chdir(config.HLS)

    subprocess.run(["v++", 
                    "-g", # debug flag
                    "-c", # compile flag
                    "-t", 
                    config.TARGET, 
                    "--platform", 
                    f"{config.XILINX_VCK5000_GEN4X8_XDMA}",
                    "-k",
                    f"{kernel_name}",
                    "-o", 
                    f"{config.HLS}/{kernel_name}/{config.TARGET}/{kernel_name}.{config.TARGET}.xo", # kernel object is saved in a target-dependent directory 
                    f"{config.HLS}/{kernel_name}/src/{kernel_name}.cpp"], # cpp file to compile
                    env=env)