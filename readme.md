Usually, if you wanted to create a beautiful but complex font for your game, you would have to make a font atlas in 
photoshop (or similar).

## So what's the Problem?
Atlasses take up a ton of space on the disk and memory in RAM. It's also difficult to tell which you will need and
what size your atlas should be (HD or low-res). Dealing with distance field fonts is a pain, so what can you use instead?

## Why FontEffects?
FontEffects allows you to [b]generate fonts on the fly in-game[/b]. 
Feed the library a picture of a letter in any size from FreeType and you'll get a beautiful symbol on return.
All of this is done with only a couple of lines of code.
The library is written in C++, is lightweight, available on the MIT license, doesn't depend on other libraries and has a simple C interface.

## How do I make a complex font?
That's where the visual editor comes in. 
It doesn't use generic filters like Photoshop (Stroke/Glow/Fill), 
but instead takes advantage of a fully-featured graph editor that can create fonts of any complexity.

Video Example:

[![example](https://img.youtube.com/vi/srbNzlthj5k/0.jpg)](https://www.youtube.com/watch?v=srbNzlthj5k)

Here is an example of a saved project in the editor (and the file that you load in the game):
```
FEF2
#sigma
size:100
distance:1.05
@nodes
*1,1,0,0,0,10,10,0,0,0,0,0,0,0,0,
*50,2,0,0,0,579,21,0,0,0,0,1,0,0,0,
*3,4,0,4,4,144,166,0,0,0,0,0,0,0,0,
 3,000000FF,0,FFFFFFFF,.7031,FF8787FF,1,1,FF,0,.5286,.8489,33.7156,1.6017
*3,3,0,0,0,145,10,0,0,0,0,0,0,0,0,
 2,FF0000FF,.3691,00FF89FF,1,1,FF,0,0,1,0,1
*10,5,0,0,0,148,345,1.75,0,0,0,0,0,0,0,
*6,6,0,0,0,360,258,10,0,0,0,0,0,0,0,
*5,7,0,0,0,355,63,0,0,0,0,0,0,0,0,
*3,8,0,0,0,533,247,0,0,0,0,0,0,0,0,
 2,01B4FFFF,0,000000FF,1,2,00,.4707,FF,.5273,0,1,0,1
@edges
*7,2,1
*8,2,4
*1,4,3
*1,3,3
*1,5,3
*7,6,2
*3,7,1
*5,7,2
*4,7,4
*6,8,2
```

Here's pseudocode demonstrating how to work with the library:
https://github.com/frankinshtein/font-effects-lib/blob/master/test/main.cpp
```cpp
    #include "fe/fe.h"

    ...........    

    // load effects from buffer into memory
    fe_effect_bundle* bundle = fe_bundle_load(buff, size);

    // get the effect by name (the file may contain different ones)
    fe_effect *effect = fe_bundle_get_effect_by_name(bundle, "sigma");    

    // get the final node
    fe_node *out_node = fe_effect_find_node_by_type(effect, fe_node_type_out);

    //  get the source image (parameters below are taken from the FreeType structs)
    int bitmap_left = 0;
    int bitmap_top = 0;
    int src_width = src_image.w;
    int src_height = src_image.h;
    int pitch = src_image.pitch;
    const void *data = src_image.data;
    FE_IMAGE_FORMAT src_format = FE_IMG_R8G8B8A8; //use FE_IMG_A8 with freetype

    int font_size = 100;//FT_Set_Pixel_Sizes(_face, 0, font_size);

    fe_im result;
    // key lines, apply our effect to the image
    bool ok = fe_node_apply(font_size,
        bitmap_left, bitmap_top,
        src_width, src_height, 
        src_format, pitch, data,
        out_node, &result);

  // upon return, result is a buffer with the image - you can now use it for whatever you need!
```

## Download Editor
- [Editor for Windows](https://www.dropbox.com/s/4j3zfraj2p6xjqb/FontEffects_win.zip?dl=0)
- [Editor for MacOSX](https://www.dropbox.com/s/4j3zfraj2p6xjqb/FontEffects_osx.zip?dl=0)
