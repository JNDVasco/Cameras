# Cameras

# Stereolabs ZED 2
## Binary File Structure
```
Start of the file
---------------------
Width
Height
Bytes per Pixel
Step
Width Bytes
---------------------
Start of Frame 1 Data 
.
.
.
End of Frame 1 Data
---------------------
---------------------
Start of Frame n Data 
.
.
.
End of Frame n Data
---------------------
End of the File 
```

Note: `Width Bytes` is the number of bytes of one pixel row (i.e one "width) 
so to get the total  data size we can do `Width Bytes * Height`.

# Intel D435i
## Binary File Structure
```
Start of the file
---------------------
Width
Height
Bytes per Pixel
Stride in Bytes
Data Size (bytes)
---------------------
Start of Frame 1 Data 
.
.
.
End of Frame 1 Data
---------------------
---------------------
Start of Frame n Data 
.
.
.
End of Frame n Data
---------------------
End of the File 
```

Note: Width and height represent the size of the frame (1280x720 for example)
and bytes per pixel represents how many bytes one pixel uses. Stride represents
the amount of bytes of one pixel row (i.e one "width"). Data Size represents the
number of bytes of the frame, this can also be obtained by `stride * height`.


# Development Environment
 - IDE: Clion 2021.2.2
 - Compilador: VisualStudio 2019 (Version 16.0)
 - CMake: 3.20.2