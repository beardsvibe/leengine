# BGFX

We use a prebuilt libs here:

- bgfx commit `daeab5c239788537a7b7797b64d24025986a1fc6`
- bx commit `f652291131dca374de8c69bf870852b684447062`

Windows libs are configured with D3D11 only.
iOS libs are configured with OpenGL ES only.
Emscripten libs are configured with OpenGL ES only.
macOS libs are configured with OpenGL only.

We also have minor changes in headers to put everything into one include folder because we only need C99 headers.