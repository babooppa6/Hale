# Day 1

* `document_load`
* `document_save`

# Day 2

- UI
	- UI/Application
	- UI/Window
	- Font rendering.
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	- UI/Editor
	- Take the Inui backend for that, don't think about using Qt too much.
- Unicode
	+ Glyphs

# Day 3

>>> Playlist for Braid, FTL, FEZ (including the special albums), Transistor, Machinarium, Primordia, Samorost & general Floex' stuff, and Macarena.

- Font rendering
	+ Grapheme Clusters
- `DocumentSession`
	- To handmade, prepare for the fact, that multi-caret will be done via API in document:
		- `document_insert(DocumentPosition *begin, DocumentPosition *end, ...)`
- `DocumentLayout`
	- To be merged with document session
	- Required for UI/Editor to work.

# Day 4

- DocumentParser
	+ Needed to test parse scheduling through event loop
	+ Will require grammars (again JSON)

# Later

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