# eventfusion
## Multisensors calibration tools
- Our approach
In folder RGB-Event_getimg

- Kalibr
https://github.com/ethz-asl/kalibr/wiki/installation

## RGB-Event-Depth-Thermal fusion
- alignment/fusion

- registration

## Multi-modals servoing

## DVS environment setup

### DV Software

[install page](https://inivation.gitlab.io/dv/dv-docs/docs/getting-started.html)

### dv-processing

C++ libs:

[official docs](https://dv-processing.inivation.com/rel_1.7/installation.html)
**For Arch Linux**:
https://aur.archlinux.org/packages/dv-processing

python package:

**Ubuntu**: (System wide)


```bash
# after add the dv ppa
sudo apt install dv-processing-python
```

**Others**:
Use venv to install:


```bash
# navigate to root folder
python -m venv dvs
# start the env
source dvs/bin/activate
# install dv
python -m pip install numpy
python -m pip install git+https://gitlab.com/inivation/dv/dv-processing
```
Installing as a git submodule see the official docs

