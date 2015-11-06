# Notes from the Stream

This is my personal document I keep to collect ideas, links and music that happen during the streams on http://twitch.tv/martincohen for anyone wanting to check back on it.

# Stack

* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Remember the cursor positions in undo.
	+ Can we do this within one undo stream?
		+ We probably can call each of the views to store their undo into the document’s undo stream.
      + We’ll however have to be careful with views that were brought to existence during the lifetime of the document.
      + Such views will have to “copy” the undo of the view they were cloned from.
      + **It’s just much better for a view to have it’s own sync’d undo stream, that is first cloned from a different view, as we won’t save any memory, and this way we’ll just keep it separated.**
+ **Multiple cursors**
	+ Actually most of the work for multiple cursors will be done at this point, so why not finish it?
	+ The most important part is to have support for this directly on the document itself, so the undo is saved in one go, and also the insert/remove is optimized for the use case, as oposed to calling single insert for every caret change.
* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
* Limit size of a block to something not too stupid, but that'll work nicely with the stack.
+ Saving file via a shortcut (Ctrl+S)
# Fix parser not notifying about format change in views.
	- Currently, the views are not notified when parsing invalidates formats.
		- The trouble is that it’s best to do it in batches as opposed to doing it per block. But a simple batch notification can be sent spaning parsed lines in `document_parse()`.
----------------------------------------------------------------
+ Stack memory.
+ Vector removal.
	+ This should actually happen automatically as the use of vector is going to be replaced by more optimized containers.

+ **Selections**
	+ Custom DirectWrite Renderer
		* Background rectangles
		* Get rid of IDWriteTextLayout and replace the formatting with our own.
			- TextFormat should be a simple struct without dependency on the RenderTarget.
				+ Window should create the platform representation on demand.
		* Get rid of public API dependency on ID2D1Brush, only use it when drawing.
+ Command line
+ Splits (at least to see two views on one document)
+ Command scopes & configuration

# To learn

- Struct/union alignment

# Later

- Parsing
	- String interning / symbols for parser.
	- Make sure that the tokens are never overlapped. Although,... do we really need that?
		+ Might even want to add a rule, that every new token should only be placed at position >= last_token.begin
- Keep 0.5 multiplies of the base font's line height to keep the rhythm of the rendered text.

- Layout updates
	+ Document > DocumentLayout
		* We can let the DocumentLayout to compare the version number it has stored for the block with the block in the document. That way the document layout will be completely lazy, and won't have to do synchronized bookkeeping with what Document blocks.
		* When document version is near the overflow limit for the version, it'll call the DocumentLayout 
		itself to reset the version number.
		* Version numbers can be 
	+ Relayout

- Path normalization and utilities.
- Runtime encoding type info.
	- Encoding preambles
	- Encoding detection by preamble
	- Possibility to optimize/replace conversion function for Hale > X and X < Hale. For most common X (UTF-8, ISO-8859-1, Windows-1250, Windows-1252)
- Document Descriptors
	+ Will need serialization format, use Qt's JSON for now.
	+ Will need directory listing.
	+ Can we skip this and leave it for later?

- FixedGapBuffer should use a padding at the end of the block to get rid of additional branches in the code.
	+ The padding can contain some useful meta information for the block.

# Name

- hale, no!
- hale.bery

# Music

- http://music.twitch.tv/
- https://www.youtube.com/audiolibrary/music

# What we play on stream

Circa Infinity
FEZ
Primordia
4mat - Prompt
4mat - Decades
Floex - Samorost 1
Floex - Samorost 2
Floex - Samorost 3 (EP)
Floax - Machinarium

To buy:

- Tycho - Dive {Full Album}
- Ibb & Obb
- Hotline Miami 2
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

- http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf

# Timelapse List

- https://www.youtube.com/watch?v=c17k4LfLkaE
- http://music.paniq.cc/album/from-zero-to-hero
- https://soundcloud.com/martincohen/sets/handmade

## ZX

- https://www.youtube.com/watch?v=d8_m8gXjmmM (typhoon, zx)

# Platforms

- **Next**: text-mode (maybe first on Windows, then port it to POSIX/Linux?)
- https://github.com/itfrombit/osx_handmade_minimal
	- Check license on that. But hopefully can be used to learn.

# Scripting
- Use CINT for scripting. CINT is a C/C++ interpreter that can call to and be called from native code.

you can go about it one of two ways:
1) Set up "C" exports in Hale and write a configuration dll which uses them, in C/C++, compile using CL.
2) Set up "C" exports in Hale and integrate CINT to call those functions, write the configuration in Hale, calling the interpreter.

1 is less work and you'll need to set up the exports anyway if you also want to support configuration dlls written in someone's favourite language.

CINT integration would make the command-line stuff work easier to do, but I'd go with 1 and build from there. Get configuration working as a compiled DLL, then integrate CINT and make configuration working dynamically using callbacks into Hale. Then reuse CINT for the command-line, which needs all the previous, and then see about getting CINT to output massages C++ to compile into a dll again for use case 1.
 Handmade Archivist
 you'll have clearly defined targets each milestone, with a marked increase in usefulness. And they all build on each other :)
9m 4 seconds ago
benefit of having CINT built-in, you always have a fallback 'compiler' for configuration on any platform. Lighter dependency than LLVM as well.
 Handmade Archivist
 and using the optional compile step, you can still get the speed boost once you're done prototyping your config.

# Questions for Mr. Awesome et. al.

- How can we generate and run ASM from memory at runtime?
	- Useful for scope selector matching.
	- We can use NASM that'll be in repo bin/
	- NASM will produce .obj files for selector matching.
	- .obj files will get linked into the application.
- String interning in C/C++. Can we use built-in interning?

# Notes

- simple_print_float(float64 number, uint16 digits = 3, bool32 sign = True, bool32 sign_if_positive)
    + https://www.refheap.com/41a61e226d06278aa004ab332
- Check if there's a donation for nightbot
- TDB - optimized branch of mingw
- No StdLib
	- https://forums.handmadehero.org/index.php/forum?view=topic&catid=4&id=79
- Adding custom command to Visual Studio
	- http://stackoverflow.com/questions/6414788/how-can-i-add-a-custom-command-to-visual-studio
- Debugging Linux from Visual Studio 2015 (purposelydrifting)
	+ http://blogs.msdn.com/b/vcblog/archive/2015/04/29/debug-c-code-on-linux-from-visual-studio.aspx
- API Trace
	+ http://apitrace.github.io/
- thepotatoleague SharpLapse tool, **push to ludumdare**
	+ https://github.com/aaronhance/SharpLapse
- http://www.landley.net/qcc/
- Pages in document
	+ Form feed character \f 
	+ Also in Unicode U+21A1 ↡ downwards two headed arrow
- Cache grind
	+ http://sourceforge.net/projects/wincachegrind/

# Miblo recommends

- Peaky Blinders (Cockney accent)
- Only Connect 

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

# Fonts

- http://www.typonine.com/fonts/font-library/tesla-slab-monospace/
- http://www.typonine.com/fonts/font-library/t9-sans-mono/