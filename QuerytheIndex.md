# Introduction #

In the [previous page](http://code.google.com/p/ogr2elasticsearch/wiki/AdvancedIndexing), we saw how to quickly index data without any modifications made to the textual data going in to the index.

# Curl Query #
Run the following command and see the results from the data stored in the index.
```
curl -POST 'http://localhost:9200/us_cities_vs._us_county_warning_areas/Feature/_search?pretty=true' -d '{
  "query": {
    "match_all": {}
  },
  "facets": {
    "tag": {
      "terms": {
        "field": "FEATURE",
        "all_terms": true
      }
    }
  }
}'
```

You can see that the top N results were returned but the facets look kind of funny!?!
```
facets: {
tag: {
_type: terms
missing: 0
total: 74483
other: 37070
terms: [
{
term: county
count: 33995
}
{
term: parish
count: 745
}
{
term: saint
count: 504
}
{
term: jefferson
count: 458
}
{
term: washington
count: 398
}
{
term: san
count: 357
}
{
term: montgomery
count: 314
}
{
term: lake
count: 305
}
{
term: jackson
count: 178
}
{
term: census
count: 159
}
```
## Next ##
> [Modify the Index Mapping](http://code.google.com/p/ogr2elasticsearch/wiki/ModifyingtheIndex)