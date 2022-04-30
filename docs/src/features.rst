Features
========

Text mode
---------

The following ThingSet functions are fully implemented:

- GET and FETCH requests (first byte ``?``)
- PATCH request (first byte ``=``)
- POST request (first byte ``!`` or ``+``)
- DELETE request (first byte ``-``)
- Execution of functions via callbacks to certain paths or via executable objects
- Authentication via callback to ``auth`` object
- Sending of publication messages (``# {...}``)
- Setup of publication channels (enable/disable, configure data objects to be published, change
  interval)

In order to reduce code size, verbose status messages can be turned off using the
``TS_VERBOSE_STATUS_MESSAGES = 0`` in ``ts_config.h``.

Binary mode
-----------

The following functions are fully implemented:

- GET and FETCH requests (function codes ``0x01`` and ``0x05``)
- PATCH request (function code ``0x07``)
- Publication of statements (function code ``0x1F``)

For an efficient implementation, only the most important CBOR data types are supported:

- Unsigned int up to 64 bit
- Negative int up to 64 bit
- UTF8 strings of up to 2^16-1 bytes
- Binary data of up to 2^16-1 bytes
- Float 32 bit
- Simple values true and false
- Arrays of above types

Currently, the following data type is still missing in the implementation.

- Float 64 (double)

It is possible to enable or disable 64 bit data types to decrease code size using the
``TS_64BIT_TYPES_SUPPORT`` flag in ``ts_config.h``.
