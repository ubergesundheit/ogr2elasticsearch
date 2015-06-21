# Introduction #

In the [previous example](http://code.google.com/p/ogr2elasticsearch/wiki/QuerytheIndex) we saw how to generate the mapping file but our query results didn't make all that much sense. To fix that we need to change the way the text is analyzed by the index.

# Analyzing Text #

There are a wide variety of things you can [do with the indices](http://www.elasticsearch.org/guide/reference/api/) but for the moment, let's just focus on getting some decent facets return from our query.

First, lets recreate the index but this time let's make the index name a little easier to work with. Generate your mapping file again.
```
ogr2ogr -progress --config ES_WRITEMAP /path/to/file/map.txt -f "ElasticSearch" http://localhost:9200 us_cities_vs._us_county_warning_areas.shp
```

Next, lets make it so that our facets are returned as un-analyzed tokens. Add "index": "not\_analyzed" to any text fields you would like to make in to facets. I will do this for FEATURE and COUNTY.
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
                "type": "string",
                "index": "not_analyzed"
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
                "type": "string",
                "index": "not_analyzed"
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

Now, rerun the full import but this time, let's specify the standard ogr2ogr -nln flag and call our index "county"
```
ogr2ogr -f "ElasticSearch" http://localhost:9200 us_cities_vs._us_county_warning_areas.shp -progress --config ES_META /path/to/file/map.txt --config ES_BULK 100000 -nln county
```

We can now run the same query again but with proper faceted results that represent the full county names in this data set.
```
curl -POST 'http://localhost:9200/county/Feature/_search?pretty=true' -d '{
  "query": {
    "match_all": {}
  },
  "facets": {
    "tag": {
      "terms": {
        "field": "COUNTY",
        "count": 10
      }
    }
  }
}'
```

Note that we now have the full county names and their frequency occurrence that we can use to reverse query the index. This is known as [faceted searching](http://en.wikipedia.org/wiki/Faceted_search).
```
facets: {
tag: {
_type: terms
missing: 0
total: 35432
other: 32956
terms: [
{
term: Jefferson County
count: 426
}
{
term: Washington County
count: 390
}
{
term: Montgomery County
count: 314
}
{
term: Jackson County
count: 259
}
{
term: Franklin County
count: 248
}
{
term: Lincoln County
count: 221
}
{
term: Lake County
count: 193
}
{
term: Wayne County
count: 169
}
{
term: Madison County
count: 129
}
{
term: Marion County
count: 127
}
]
}
}
}
```