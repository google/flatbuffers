# Loading Flatbuffer Generated Wireshark Plugins
Copy the contents of this directory into your chosen Wireshark plugins folder. make sure to point `flatc` at the same plugins folder when generating with `--wireshark`.

> **Note:** If you place them in a folder, ensure the folder name appears earlier in ASCII order than any generated folders.

## Why Are the Wireshark Files Named Strangely?

According to the [Wireshark documentation](https://www.wireshark.org/docs/wsdg_html_chunked/wsluarm.html), the file load process and order are critical:

> ... **all files ending with .lua** are loaded from the global plugins directory and its subdirectories. Then all files ending with .lua in the personal Lua plugins directory and its subdirectories are loaded. **The files are processed in ASCIIbetical order (compared byte-by-byte, as strcmp), descending into each subdirectory depth-first in order."**

To ensure the correct load order, generated files are named according to the below table.

| File Prefix | Description                                                   |
|-------------|---------------------------------------------------------------|
| `00_`       | Base and helper FlatBuffers files needed by generated code    |
| `10_`       | Generated definitions file containing important lookup tables |
| `20_`       | Generated table and struct files that are not root tables     |
| `30_`       | Generated tables marked as root tables                        |