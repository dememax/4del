####################
Sphinx essentials
####################

My Quick help on Shpinx.

The main page: https://www.sphinx-doc.org/

**********
Sections
**********

* ``#`` with overline, for parts
* ``*`` with overline, for chapters
* ``=`` for sections
* ``-`` for subsections
* ``^`` for subsubsections
* ``"`` for paragraphs

************
Code block
************

.. code-block:: rst
   :caption: Adding code block sample

   .. code-block:: bash
      :caption: Adding code block sample

      export A=b

For languages supported: https://pygments.org/docs/lexers/

***************
Images
***************

.. code-block:: rst
   :caption: Introducing an image from a local Jpeg file

   .. image:: ./IMG_20250505_144207\ Guillaume\ -\ Yocto\ 3\ layers\ -\ v4l2.jpg
      :alt: Guillaume - Yocto 3 layers - v4l2 - QDMA
      :align: center

===========
Names
===========

If I use the file name with spacese as it is:

.. code-block:: rst
   :caption: Introducing an image from a local Jpeg file

   .. image:: ./IMG_20250505_144207 Guillaume - Yocto 3 layers - v4l2.jpg

This gives an error:

.. code-block::
   :caption: When file name with spaces is used as it is

   ..../docs/index.rst:8: WARNING: image file not readable: IMG_20250505_144207Guillaume-Yocto3layers-v4l2.jpg [image.not_readable]

Only backslash (\\) escape is working for spaces in file names.

From a note at https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html#escaping-mechanism

   In contexts where the parser expects a URI-reference
   (the link block of external hyperlink targets or
   the argument of an "image" or "figure" directive),
   **whitespace is ignored** by default.

It was note 2 for

   In URI context [2], backslash-escaped whitespace represents a single space.

I've tried with Sphinx 8.1.3 two other methods,
but unfortunately they don't work:

#. Use quotes around the file path:

   Wrap the file path in quotes, like this:

   .. code-block:: rst
      :caption: Using quotes for the file name with spaces

      .. image:: "./IMG_20250505_144207 Guillaume - Yocto 3 layers - v4l2.jpg"

      ..../docs/index.rst:8: WARNING: image file not readable: "./IMG_20250505_144207Guillaume-Yocto3layers-v4l2.jpg" [image.not_readable]

#. Use URL encoding to represent special characters in the file path.
   For example, a space would be represented as %20:

   .. code-block:: rst
      :caption: Using URL encoding for the file name with spaces

      .. image:: ./IMG_20250505_144207%20Guillaume%20-%20Yocto%203%20layers%20-%20v4l2.jpg

      ..../docs/index.rst:8: WARNING: image file not readable: IMG_20250505_144207%20Guillaume%20-%20Yocto%203%20layers%20-%20v4l2.jpg [image.not_readable]

*******************************
Put a link to the local doc
*******************************

It should be a rst document, so, to have \*.rst extension.

The name of file can contain spaces, not like for image names,
say, you've got '09-51 Ubuntu apt - Phasing - Asking Meta.rst',
you need only omit the extension of the file:

.. code-block:: rst
   :caption: Including a link to the local document

   :doc:`./09-51 Ubuntu apt - Phasing - Asking Meta`

In the text, the title will be displayed, not the file name.