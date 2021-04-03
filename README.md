# Resize
A program to resize BMP images by a factor given by you.

This program allows you to resize BMP images - compile the file using ```gcc -o resize resize.c``` and then compile it with three command line arguments:
<ol>
  1. The factor by which you want to resize the image (0.5 would mean half as much while 2 would mean twice as much on each axis). 
  2. The file you want to resize.
  3. The file you want to send the resized image to.
</ol>

The output should look something like this: ```./resize 2 small.bmp large.bmp```.

When compressing an image, I unfortunately cannot keep all the original colors if the picture is not uniform to begin with - I just skip pixels vertically and horizontally depending the factor given. 
For increasing the size of images, I do the opposite and multiply each pixel horizontally and vertically by the amount of the factor.
