declare namespace itunes="http://www.itunes.com/dtds/podcast-1.0.dtd";
for $i in doc($uri)/rss/channel/item
    return fn:string-join( (
        $i/title/string(), 
        $i/itunes:author/string(),
        $i/description/string(),
        $i/itunes:subtitle/string(),
        $i/link/string(),
        $i/guid/string(),
        $i/pubDate/string(),
        $i/itunes:duration/string()
        ), " %%QT_DEMO_DELIM%% ")
