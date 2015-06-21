# Introduction #

This driver follows the standard [ogr2ogr](http://www.gdal.org/ogr2ogr.html) usage convention. Follow [this tutorial](http://www.elasticsearch.org/guide/reference/setup/installation.html) to get your instance of ElasticSearch up and running.

# Usage #

**Basic Transform:**
```
ogr2ogr -progress -f "ElasticSearch" http://localhost:9200 my_shapefile.shp
```

**Create a Mapping File:** <br />
The mapping file allows you to modify the mapping according to the [ElasticSearch field-specific types](http://www.elasticsearch.org/guide/reference/mapping/core-types.html). There are many options to choose from, however, most of the functionality is based on all the different things you are able to do with text fields.
```
ogr2ogr -progress --config ES_WRITEMAP /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 my_shapefile.shp
```

**Read the Mapping File:** <br />
Reads the mapping file during the transformation...
```
ogr2ogr -progress --config ES_META /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 my_shapefile.shp
```

**Bulk Uploading (for larger datasets):** <br />
Bulk loading helps when uploading a lot of data. The integer value is the number of bytes that are collection before being inserted.
```
ogr2ogr -progress --config ES_BULK 10000 -f "ElasticSearch" http://localhost:9200 PG:"host=localhost user=postgres dbname=my_db password=password" "my_table" -nln thetable
```

**Overwrite the current Index:** <br />
If specified, this will overwrite the current index. Otherwise, the data will be appended.
```
ogr2ogr -progress --config ES_OVERWRITE 1 -f "ElasticSearch" http://localhost:9200 PG:"host=localhost user=postgres dbname=my_db password=password" "my_table" -nln thetable
```

**Specify several at a time:** <br />
Several flags can be set at the same time.
```
ogr2ogr -progress --config ES_OVERWRITE 1 --config ES_BULK 10000 --config ES_META /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 PG:"host=localhost user=postgres dbname=my_db password=password" "my_table" -nln thetable
```