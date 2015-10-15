# Day 1

* `document_load`
* `document_save`
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- `DocumentSession`
	- To handmade, prepare for the fact, that multi-caret will be done via API in document:
		- `document_insert(DocumentPosition *begin, DocumentPosition *end, ...)`
- `DocumentLayout`
	- To be merged with document session
	- Required for UI/Editor to work.
- Path normalization and utilities.

# Day 2

- Remainders of Day 1
- UI/Application
- UI/Window
- UI/Editor

# Day 3

- DocumentParser
	+ Needed to test parse scheduling through event loop
	+ Will require grammars (again JSON)

# Later

- Runtime encoding type info.
	- Encoding preambles
	- Encoding detection by preamble
- Document Descriptors
	+ Will need serialization format, use Qt's JSON for now.
	+ Will need directory listing.
	+ Can we skip this and leave it for later?
- UndoStream
	+ We can keep prototype version for a while.