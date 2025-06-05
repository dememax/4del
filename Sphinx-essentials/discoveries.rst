####################
Sphinx discoveries
####################

***************************
Queries for static files
***************************

The query string like ``url?v=xxxxxxx`` appended to static file URLs
(like CSS or JavaScript files) in Sphinx-generated HTML
is a common web development technique called cache busting.

For example:

.. code-block:: shell
   :caption: Browser requests some static CSS and JS files with a query part

   user@host ~/tmp/docs $ python3 -m http.server 9000
   Serving HTTP on 0.0.0.0 port 9000 (http://0.0.0.0:9000/) ...
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET / HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /_static/pygments.css?v=d1102ebc HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /_static/alabaster.css?v=12dfc556 HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /_static/documentation_options.js?v=4d1abcb2 HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /_static/basic.css HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /_static/favicon.ico HTTP/1.1" 200 -
   192.168.1.26 - - [04/Jun/2025 12:31:19] "GET /intro.html HTTP/1.1" 200 -

In the context of Sphinx:

* The ``v=code`` part is a version identifier. This ``code`` is often:

  * A short hash of the file's content.
  * A version number derived from the Sphinx version or the theme version.

* When Sphinx or its theme (like 'alabaster' ) is updated,
  or when critical static files it manages are changed,
  this version identifier is updated.
* This ensures that users Browse your documentation will
  receive the latest versions of these static assets
  (like stylesheets or JavaScript for the search function)
  when you rebuild and deploy your documentation,
  preventing issues caused by stale cached files.
