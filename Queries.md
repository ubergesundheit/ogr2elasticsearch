# Introduction #

By far the easiest way to test your ElasticSearch index out is to use Curl.

# Geo-Distance (Point Radius) #

```
curl -XGET 'http://localhost:9200/index/FeatureCollection/_search?pretty=true' -d '{
    "query": {
        "filtered": {
            "query": {
                "match_all": {}
            },
            "filter": {
                "geo_distance": {
                    "distance": "100km",
                    "geometry.coordinates": [17.006749,81.726275]                   
                }
            }
        }
    }
}'
```
