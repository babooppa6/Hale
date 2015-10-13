# Hale

Prototype turning into an open source text editor.

## 11 Days Marathon

From *Thursday; October 16, 2015* to *Monday; October 26, 2015* on [Twitch](http://www.twitch.tv/martincohen).

I plan to spend these eleven days on finishing the alpha release.

## Links

- [Code on **GitHub.**](https://github.com/martincohen/Hale)
- [Rants and News on **Twitter.**](http://twitter.com/martin_cohen)
- [Streaming on **Twitch.**](http://twitch.com/martincohen)
- [Support on **Patreon.**](http://patreon.com/martincohen)

## Codebase

Codebase currently actually contains two versions of Hale.

- **prototype** -- done in OOP and Qt. I found Qt to be particularly great for quick prototyping of the features and the internal architecture. However, GPL, mandatory OOP and a ton of build dependencies makes it inappropriate for my goals.
- **handmade** -- done without OOP, still has some dependencies (for example `hale::vector` internally uses `std::vector`), but the goal is to get rid of them.

## Notation

There is some notation used in the following document.

### State notation

- `P` in *prototype*
- `H` in *handmade*
- `½` partially in *handmade*
- `*` not yet implemented

### Planning notation

- `α` alpha release (hopefully after the 11 days marathon, some features may be scaled down to their simpler versions)
- `β` beta release
- `γ` gamma release

## Goals

Some of these goals are motivated by my desire to learn and try new things. The most important goal is to provide a good and reasonable software that is written for hardware and with love.

Majestic goals follow (disclaimers apply):

- **MIT license** - Good license to use in various legal settings.
- **No build system** - Just a `.bat` and shell scripts. Most of the build configuration in `.h` files.
- **No dependencies** - Goal is to not depend on any library. Standard library as an optional dependency.
- **No OOP** - It's not about neglecting OOP, but mostly about learning to see the code and the data differently.
- **Use as component** - Easy to embed whole or in parts in other code bases.
- **Text and GUI** - Text-mode is not dead. Definitely not in today's world of UX targeted at occasional users.
- **Multiple OS** - Windows at first (α), Linux and Mac (β) will follow.

## Component vs. Standalone

There is a bit of ambiguity in my goals. It's correct that I want Hale to be both, a library and an application. However I believe the best way to create a library is to first make code that uses it. Therefore I focus to deliver an application at first.

By example: If you are familiar with [Scintilla Editing Component](http://www.scintilla.org/) (a popular text editing component), you might also know [SciTE - Scintilla Text Editor](http://www.scintilla.org/SciTE.html) (example editor using Scintilla made by the same author).

## Modules

The codebase is split into logical modules.

- `½|α` **Platform** - Platform layer support for Windows.
- `H|α` **Foundation** - Basic types and general-purpose containers and operations with them.
- `H|α` **Document** - Editing and managing documents.
- `P|α` **Scope** - Scope-based configuration.
- `P|α` **Bracket** - Directory-based configuration.
- `P|α` **Shell** - Shell integration.
- `P|α` **UI/Application** - Top-level UI management.
- `P|α` **UI/Window** - Top-level windows.
- `P|α` **UI/Panel** - Window layout.
- `P|α` **UI/Model** - Glue between models (Document, Bracket, Shell) and UI.
- `½|α` **Lua** - Lua scripting and configuration.

# Features

Although I don't want the core of the editor to not contain any "real" features, but rather a foundation for building them, there are still some properties of the system, that can be propagated.

Some other features come from the prototype itself and are lesser versions of what I would like to achieve. However, in order to deliver a working product as soon as possible, I use them as a good placeholder.

- `P|α` **Platform font rendering** - Or better: optional platform font rendering. [ClearType](https://en.wikipedia.org/wiki/ClearType), for instance, doesn't deal well with some great fonts like [Fira Mono](https://mozilla.github.io/Fira/) or [Nitti](https://www.boldmonday.com/typeface/nitti/). [FreeType](http://www.freetype.org/), [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) or [distance field font rendering](http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf) might do a much better job (usually thanks to it's configuration). User has to have that choice for optimal reading experience.
- `½|α` **Unicode support** - 7 bits is not enough. This document itself uses a good portion of Unicode blocks, even though it's entirely in (a bit broken, but) English.
- `½|α` **Multiple cursors** - Editing with multiple cursors and selections at once.
- `*|β` **Large files** - Opening and editing extremely large files (16 exbibytes; 2⁶⁴ bytes)
- `½|α` **Syntax annotation and highlighting** - Currently via TextMate-like grammars, however custom parsers are planned to replace this functionality.
- `P|α` **Brackets** - A different take on projects and project-based configuration (along with Scope-based configuration).
- `P|α` **Scope-based configuration** - Allows you to set different visual and behavioral configuration based on current state of application, window, panel, model, bracket or document.
- `P|α` **Shell integration** - Specialized terminal emulator embedded into the application.
- `½|α` **Lua scripting** - A general purpose scripting for prototyping and makeshift extensions.

To not spoil it all, here is a few secret features.

- `*|β` **C++ Scribe** - A secret feature.
- `*|β` **Streams** - A secret feature.