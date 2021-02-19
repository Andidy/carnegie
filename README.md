# carnegie

Originally when I was learning DX12 API I followed examples that produced 3D images.

October 16 2020 Screenshot:
![example screenshot of program running in october 2020](./screenshots/Oct16-2020.PNG)

But I wanted to make a 2D tilemap system, so I implemented that and began iterating on it. This google doc explains the shader technique I used and some of the surrounding vocabulary. Its accurate to the first video and second screenshot.
https://docs.google.com/document/d/1knAz5btbuFTvYX5yFg6tosZ_1g7FeHanlujyFwCRAWI/edit?usp=sharing

Animation first working: Nov10: https://youtu.be/KzrKdUyHgr4

November 24 2020 Screenshot:
![example screenshot of program running in november 2020](./screenshots/Nov24-2020.PNG)

After I had the tilemaps, layering, and animation working, I wanted to allow the user to move one of the characters around with the keyboard.
I added movement between the tiles and then worked on making this movement prettier than just pressing a key and immediately the character was in the next tile.

Lerping between tiles: Dec6: https://youtu.be/RJPII_JdBQE

Moving character with user input and making the lerp smooth: Dec15: https://youtu.be/MC1S00vQXRo

---
Requirements:
- Windows 10 SDK 10.0.19041.0
- A command prompt with MSVC x64 in path
- To compile and run
```
> build.bat
> run.bat
```
