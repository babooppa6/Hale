- 
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Editing
	+ Platform support for keyboard
		* Partial support for scopes?
	+ Cursors
	+ Layout updates
		* Document > DocumentLayout
		* Relayout
- Scrolling
- TextFormat should be a simple struct without dependency on the RenderTarget.
	+ Window should create the platform representation on demand.

- Window layout/panel system
- Parsing

# Later

- DocumentParser
	+ Needed to test parse scheduling through event loop
	+ Will require grammars (again JSON)
- Path normalization and utilities.
- Runtime encoding type info.
	- Encoding preambles
	- Encoding detection by preamble
	- Possibility to optimize/replace conversion function for Hale > X and X < Hale. For most common X (UTF-8, ISO-8859-1, Windows-1250, Windows-1252)
- Document Descriptors
	+ Will need serialization format, use Qt's JSON for now.
	+ Will need directory listing.
	+ Can we skip this and leave it for later?
- UndoStream
	+ We can keep prototype version for a while.

# Name

- hale, no!
- hale.bery

# Music

- http://music.twitch.tv/
- https://www.youtube.com/audiolibrary/music

# What we play on stream

Monument Valley
The Floor is Jelly
FEZ
FTL
Risk of Rain

# Timelapse

youtube-dl -R infinite -c --hls-prefer-native http://www.twitch.tv/martincohen/v/21604211

http://www.labnol.org/internet/useful-ffmpeg-commands/28490/
ffmpeg -i movie.mp4 -r 1.0 frames_%08d.png
d:\progs\ffmpeg\bin\ffmpeg -i "Hale (text editor) Document Rendering-v21604211.mp4" -r 10.0 frames/frames_%08d.png

# Questions

- Going without standard libraries. (Platform46)
- http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf

# Notes

- http://www.landley.net/qcc/
