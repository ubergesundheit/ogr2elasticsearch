# Introduction #

ElasticSearch is an easy to use and very scalable search engine. The OGR2ElasticSearch project is intended to allow the conversion from any of the supported <a href='http://www.gdal.org/ogr/ogr_formats.html'>OGR</a> formats to an ElasticSearch single node or cluster.

For a full list of OGR formats, please click here: http://www.gdal.org/ogr/ogr_formats.html

This is just the beginning but I intend to eventually store the data so that it can be retrieved as [GeoJSON](http://geojson.org/) for use in most modern mapping applications.

# Getting Started #

1.  Check out the source code <a href='http://code.google.com/p/ogr2elasticsearch/source/checkout'>here</a>

  * These sources will give you the files needed to add a new driver to build against the latest version of GDAL.

2. Now check out GDAL from trunk:
```
svn checkout https://svn.osgeo.org/gdal/trunk/gdal gdal
```

3. The files you need are located in their respective directories. Start at GDAL root directory and add the following
  * ogr/
    * ogrsf\_frmts/
      * GNUmakefile
      * makefile.vc
      * ogrsf\_fmts.h
      * elastic/
        * GNUmakefile
        * makefile.vc
        * ogr\_elastic.h
        * ogrelasticdatasource.cpp
        * ogrelasticdriver.cpp
        * ogrelasticlayer.cpp
      * generic/
        * GNUmakefile
        * makefile.vc
        * ogr\_attrind.cpp
        * ogr\_gensql.h
        * ogr\_gensql.cpp
        * ogr\_miattrind.cpp
        * ogrlayer.cpp
        * ogrregisterall.cpp
        * ogrsfdriver.cpp
        * ogrfdriverregistrar.cpp

4. Now that the files are in the right place, <a href='http://www.adamestrada.net/2011/08/22/how-to-build-gdal/'>click here</a> for instructions on how to build GDAL.<br />
5.  **Note:** the only dependency is libcurl.

## [Usage Examples](http://code.google.com/p/ogr2elasticsearch/wiki/PageName) ##