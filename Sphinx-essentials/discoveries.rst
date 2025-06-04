####################
Sphinx discoveries
####################

***************************
Queries for static files
***************************

The query string like "url?v=xxxxxxx" appended to static file URLs
(like CSS or JavaScript files) in Sphinx-generated HTML
is a common web development technique called cache busting.

In the context of Sphinx:

* The v=code part is a version identifier. This "code" is often:

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
