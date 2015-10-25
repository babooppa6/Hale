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
