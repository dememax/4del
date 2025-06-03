#################
Sphinx essentials
#################

My Quick help on Shpinx.

The main page: https://www.sphinx-doc.org/

Sections
********

* ``#`` with overline, for parts
* ``*`` with overline, for chapters
* ``=`` for sections
* ``-`` for subsections
* ``^`` for subsubsections
* ``"`` for paragraphs

Code block
**********

.. code-block:: rst
   :caption: Adding code block sample

   .. code-block:: bash
      :caption: Adding code block sample

      export A=b

For languages supported: https://pygments.org/docs/lexers/

*************
Images
*************

.. code-block:: rst
   :caption: Introducing an image from a local Jpeg file

   .. image:: ./IMG_20250505_144207\ Guillaume\ -\ Yocto\ 3\ layers\ -\ v4l2.jpg
      :alt: Guillaume - Yocto 3 layers - v4l2 - QDMA
      :align: center

===========
Names
===========

Only backslash (\\) escape is working for spaces in file names.

From a note at https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html#escaping-mechanism

   In contexts where the parser expects a URI-reference
   (the link block of external hyperlink targets or
   the argument of an "image" or "figure" directive),
   **whitespace is ignored** by default.

It was note 2 for

   In URI context [2], backslash-escaped whitespace represents a single space.

I've tryed with Sphinx 8.1.3 two other methods,
but unfortunately they don't work:

#. Use quotes around the file path:

   Wrap the file path in quotes, like this:

   .. code-block:: rst
      :caption: Using quotes for the file name with spaces

      .. image:: "path/to/image with spaces.png"

#. Use URL encoding to represent special characters in the file path.
   For example, a space would be represented as %20:

   .. code-block:: rst
      :caption: Using URL encoding for the file name with spaces

      .. image:: path/to/image%20with%20spaces.png
