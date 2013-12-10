for $i in doc($uri)/rss/channel/item
    return fn:string-join( (
        $i/title/string(), 
        "author",
        $i/description/string(),
        "subtitle",
        $i/link/string(),
        $i/guid/string(),
        $i/pubDate/string(),
        "duration"
        ), " %%QT_DEMO_DELIM%% ")
