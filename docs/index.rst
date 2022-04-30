====================================
ThingSet C/C++ Library Documentation
====================================

The ThingSet protocol provides a consistent, standardized way to configure, monitor and control
ressource-constrained devices via different communication interfaces.

Protocol specification: `thingset.io <https://thingset.io>`_

The specification is developed in an open source way and still in progress. You can contribute
ideas for improvement. This library implements v0.5 of the specification.

The implementation is tested vs. newlib-nano and the default newlib provided by GNU C++ compiler.

The library can be integrated into `Zephyr RTOS`_ projects as a module.

This implemntation uses the very lightweight JSON parser [JSMN](https://github.com/zserge/jsmn).

This documentation is licensed under the Creative Commons Attribution-ShareAlike 4.0 International
(CC BY-SA 4.0) License.

.. image:: static/images/cc-by-sa-centered.png

The full license text is available at `<https://creativecommons.org/licenses/by-sa/4.0/>`_.

.. _Zephyr RTOS: https://zephyrproject.org
.. _ThingSet: https://thingset.io

.. toctree::
    :caption: Overview
    :hidden:

    src/features

.. toctree::
    :caption: Development
    :hidden:

    src/dev/usage
    src/dev/unit_tests

.. toctree::
    :caption: API Reference
    :hidden:

    src/api/library
    src/api/internal
