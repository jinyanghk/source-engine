# DISPLAY=:0 ~/.venv/bin/python3 ../jin/remote.py
# DISPLAY=:0 needs to be outside script

import os
import subprocess
import time
import pyguitest

env = os.environ.copy()
#env["DISPLAY"] = ":0"
env["LD_LIBRARY_PATH"] = "bin" + os.pathsep + env.get("LD_LIBRARY_PATH", "")

proc = subprocess.Popen(
    ["./hlmv", "-w", "1920", "-h", "1080"],
    env=env
)

time.sleep(5)  # give enough time for app to start

gui = pyguitest.connect()

subprocess.run(
    ["scrot", "desktop.png"],
    #env=env
)
print("desktop snapshot saved")

time.sleep(5) 

# optionally shut down app
proc.terminate()