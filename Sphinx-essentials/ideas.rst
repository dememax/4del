#################
Sphinx ideas
#################

************
Sections
************

From Sphinx docs:

    Section headers (ref) are created by underlining (and optionally overlining)
    the section title with a punctuation character, at least as long as the text

========================
Optional overlining
========================

The overlining must be used to have more context for diffs.

===================================
At least as long as the text
===================================

The length of overlining and underlining must be bigger than the text
to not change it each time the text grows.

In addition, to differentiate have an idea on how the sections were created,
the length should grow each time making a kind of the "Christmas tree":

.. code-block:: rst
   :caption: Christmas tree of overlining and underlining in the doc

   ######

   **********

   ============

   --------------------------
