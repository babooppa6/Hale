# Processing

- https://r12a.github.io/scripts/tutorial/

# Unicode

The goal is to get the Unicode meta information to be able to get grapheme clusters (the basics of cursor movement), as well as requirements for the font.

Each document will have a unicode table associated with it. That can also be a function that in the simplest case will just index the (private) static table:

- ChInfo *CharacterMetaInfo(u32 character);

We can support dynamic range of blocks, depending on what codepoints are present in the document.
Typically 99% of time would be spent on BMP plane (162 blocks; 0000-ffff), from that a ton of time in programming is spent only within 20-7F.

Also the line endings can be simply stored in a similar table. We don't really need to know whether it's CR+LF or LF+CR, we can base all our detection on the amounts of CR and LF characters.

- If CR == LF > CR+LF
- If CR > LF

Blocks are always aligned to 16 bytes, so from BPM, there's 4096 16-byte long blocks. We can encode this information during the file decoding time in 4096 bits (512 byte long static array, 8 64-bit slots). That would get us a basic coverage of characters.

IDEA: We can store this information per-fixed-block so the movement within the block can be optimized to maximum. However, it'll possibly be sufficient to store information whether the block contains character above 7F or above FFFF.

IDEA: Implement UTF-8 and 7F readers so we can read the data from the blocks directly from the memory without ever converting. However, once a write is made, the block has to be copied (as it'll most likely be mmap'd), in that case we'll simply convert the block to Hale.

IDEA: We don't have to copy the whole block, we can just move the gap end and start pointers to virtually "delete" stuff from the mmap'd blocks, and we will copy only the part we have "deleted".

IDEA: When loading, mmap the file to memory, show and parse it immediately and start copying/converting the blocks. This strategy can be used for loading from slower media, or for loading mid-sized files (10-100MB).

From that we can scale from static tables to trie. And also implement only the 7F or BMP at first and scale up.

- 007F static
	- **Basic Latin**
- 00FF static 
	- **Latin-1 Supplement, ISO-8859-1**
	- French
    - German
    - Spanish
    - Icelandic
    - Vietnamese
- 017F static
	- **Latin Extended-A**
    - Czech
    - Dutch
    - Polish
    - Turkish
- 024F static
	- **Latin Extended-B**
    - Africa alphabet
    - Pan-Nigerian
    - Americanist
    - Khoisan
    - Pinyin
    - Romanian
...

NOTE: While only in the Basic Latin there's 16 blocks already taken which we can ignore and use for something else.

# Symbol blocks

U+2000 - U+2BFF (3071 codepoints)