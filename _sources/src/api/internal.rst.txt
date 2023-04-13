Internal
========

Below functions are only used internally by the library.

.. doxygenfile:: thingset_priv.h
   :project: app


JSON (JSMN)
-----------

The JSON parser is using the `JSMN library <https://github.com/zserge/jsmn>`_.

.. doxygenfile:: jsmn.h
   :project: app

CBOR
----

The internal CBOR parser is very light-weight and contains only the features required for the
ThingSet library.

.. doxygenfile:: cbor.h
   :project: app
