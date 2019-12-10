# Wi-Fi coverage emulator

A simple Wi-Fi coverage emulator created under CMC MSU computer graphics course.
For task requirements see *PRD.pdf*.

## Build

Open project solution with Visual Studio. Make sure that you installed C++ ATL for your build tools.

Build solution using `Release x64` configuration. You'll get executable at `bin/`.

Put `flat.obj` and `config.txt` from `src/` in the same folder as executable. You can use other configs or even create your own.

Run from command line:

```cmd
RayTracer.exe config.txt
```

## Results

![cmd](docs/images/Screenshot&#32;1.png)

config, power = 100

![room](img/config,%20power%20=%20100.png)

config, power = 150

![room](img/config,%20power%20=%20150.png)

config 3, power = 200

![room](img/config3,%20power%20=%20200.png)
