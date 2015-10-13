# Hale

Prototype turning into an open source text editor.

Majestic goals follow (disclaimer apply):

- (✓) **MIT license** - Good license to use in various legal settings.
- **No build system** - Just a `.bat` and shell scripts. Most of the build configuration in `.h` files.
- **No dependencies** - Goal is to not depend on any library. Standard library as an optional dependency.
- **Use as component** - Easy to embed whole or in parts in other code bases.
- **Text and GUI** - Text-mode is not dead. Definitely not in today's world of UX decay.


# Prototype

Prototype is done in OOP and Qt. I found Qt to be particulary great for quick prototyping of the features and the internal architecture. However, GPL, mandatory OOP and a ton of build dependencies makes it inappropriate for my goals.

Most of the functionality is currently in this branch of the code. All the files that are not prefixed with `hale_` are prototype files, where there's a little done for the code quality and optimizations. Current functionality in prototype:

- 

# Handmade

Handmade part is all files that with `hale_` prefix. This code is slowly eating parts of the prototype.
