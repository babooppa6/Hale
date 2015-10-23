- The document_layout_layout should note the whole range of lines that has been layouted, so that the draw operation will only draw what has been laid out.


* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Scrolling
- Custom DirectWrite Renderer
	+ Background rectangles
	+ Get rid of IDWriteTextLayout and replace the formatting with our own.
		* TextFormat should be a simple struct without dependency on the RenderTarget.
			- Window should create the platform representation on demand.
	+ Get rid of public API dependency on ID2D1Brush, only use it when drawing.

- Window layout/panel system
- Parsing
- Keep 0.5 multiplies of the base font's line height to keep the rhythm of the rendered text.

simple_print_float(float64 number, uint16 digits = 3, bool32 sign = True, bool32 sign_if_positive)
https://www.refheap.com/41a61e226d06278aa004ab332

# Later

- Layout updates
	+ Document > DocumentLayout
		* We can let the DocumentLayout to compare the version number it has stored for the block with the block in the document. That way the document layout will be completely lazy, and won't have to do synchronized bookkeeping with what Document blocks.
		* When document version is near the overflow limit for the version, it'll call the DocumentLayout 
		itself to reset the version number.
		* Version numbers can be 
	+ Relayout

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

[BOUGHT] Circa Infinity
[BOUGHT] FEZ
[BOUGHT] Primordia
4mat - Prompt
4mat - Decades

To buy:

- Antichamber
- Monument Valley
- The Floor is Jelly
- FTL
- Risk of Rain
- Mirror's Edge
- Starbound

# Timelapse

youtube-dl -R infinite -c --hls-prefer-native http://www.twitch.tv/martincohen/v/21604211

http://www.labnol.org/internet/useful-ffmpeg-commands/28490/
ffmpeg -i movie.mp4 -r 1.0 frames_%08d.png
d:\progs\ffmpeg\bin\ffmpeg -i "Hale (text editor) Document Rendering-v21604211.mp4" -r 10.0 frames/frames_%08d.png

# Questions

- Going without standard libraries. (Platform46)
- http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf

# Notes

- Debugging Linux from Visual Studio 2015 (purposelydrifting)
	+ http://blogs.msdn.com/b/vcblog/archive/2015/04/29/debug-c-code-on-linux-from-visual-studio.aspx
- thepotatoleague SharpLapse tool, **push to ludumdare**
	+ https://github.com/aaronhance/SharpLapse
- http://www.landley.net/qcc/
- Pages in document
	+ Form feed character \f 
	+ Also in Unicode U+21A1 ↡ downwards two headed arrow

# Animating Caret

<d7samurai> i would just store the pixels that are covered by the caret
<d7samurai> and render that and then the caret on top (with alpha)
<martincohen> hmm... okay, I'll get back to you on that (as I'm not sure I know how to do it with Direct2D, yet)
<martincohen> but I'll investigate
<d7samurai> ah.. direct2d.. i forget
<d7samurai> well, i have been though many similar considerations in my system
<d7samurai> (only i need to draw many many things that happen to be behind some semi-transparent stuff that is changing)
<d7samurai> you have something called ScissorRects that will allow you to mask areas in the rendertarget so they are not overwritten
<d7samurai> and you can make a rendertarget also be a shader resource (i.e. a texture the gpu can read from)
<d7samurai> so it's possible to do a draw call that will "copy" the area where the caret is
<d7samurai> to some other texture
