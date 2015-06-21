# Introduction #

ElasticSearch is typically not meant for querying geospatial data so what you get is the inherent search capabilities that come with the underlying [Lucene](http://lucene.apache.org/java/docs/index.html) index. The geospatial support is good but with the added bonus of doing some pretty complex querying, this application ends up being really powerful.

# Create a Mapping file #

As seen on the [Usage](http://code.google.com/p/ogr2elasticsearch/wiki/PageName) page, there is a way to generate a mapping file that can be edited before inserting the data in to the Index. I grabbed a shapefile from [Geocommons](http://www.geocommons.com) and placed it [here](http://code.google.com/p/ogr2elasticsearch/downloads/detail?name=us_cities_vs._us_county_warning_areas.zip&can=2&q=#makechanges) for testing purposes.

Download and unzip the file and then run the following command.
```
ogr2ogr -progress --config ES_WRITEMAP /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 us_cities_vs._us_county_warning_areas.shp
```

What you should get is as follows. (map.txt) Go ahead and use [JSONLint](http://jsonlint.com/) to pretty up your code making it easier to work with.
```
{
    "Feature": {
        "properties": {
            "geometry": {
                "properties": {
                    "coordinates": {
                        "store": "yes",
                        "type": "geo_point"
                    }
                }
            },
            "NAME": {
                "store": "yes",
                "type": "string"
            },
            "Y_AXIS_COO": {
                "store": "yes",
                "type": "float"
            },
            "FEATURE": {
                "store": "yes",
                "type": "string"
            },
            "POP_2000": {
                "store": "yes",
                "type": "integer"
            },
            "RANK": {
                "store": "yes",
                "type": "integer"
            },
            "POP_RANGE": {
                "store": "yes",
                "type": "string"
            },
            "FIPS55": {
                "store": "yes",
                "type": "string"
            },
            "STATE": {
                "store": "yes",
                "type": "string"
            },
            "DISPLAY": {
                "store": "yes",
                "type": "integer"
            },
            "NAME_OLD": {
                "store": "yes",
                "type": "string"
            },
            "OBJ_IDENTI": {
                "store": "yes",
                "type": "integer"
            },
            "CITIESX020": {
                "store": "yes",
                "type": "integer"
            },
            "STATE_FIPS": {
                "store": "yes",
                "type": "string"
            },
            "PLACEMENT": {
                "store": "yes",
                "type": "string"
            },
            "X_AXIS_COO": {
                "store": "yes",
                "type": "float"
            },
            "COUNTY": {
                "store": "yes",
                "type": "string"
            },
            "FIPS": {
                "store": "yes",
                "type": "string"
            },
            "RANK_DISTA": {
                "store": "yes",
                "type": "integer"
            }
        }
    }
}
```

Now run the full import on the shapefile using the --config ES\_META flag pointing to your map.txt file.
```
ogr2ogr -progress --config ES_META /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 us_cities_vs._us_county_warning_areas.shp
```

## Next ##
[Query the Index](http://code.google.com/p/ogr2elasticsearch/wiki/QuerytheIndex)