# Changelog

## Version history of node-epics-pcas

### v0.1.1

- Release Value and buffer when not used anymore in order to reduce memory leak
- Add unreference() to release array pointer when GDD is not used anymore in order to reduce memory leak

### v0.1.0

- Supports macOS platform
- Added README

### v0.0.2

- Supports Linux platform

### v0.0.1

**Initial version:**

- Implements a EPICS Portable Channel Access Server using the PCAS shared library via koffi library in Node.js
- Supports Windows platform