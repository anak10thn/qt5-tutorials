<?xml version="1.0" encoding="UTF-8"?>

<!-- 
 * This file is part of TwitterView**
 *
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
 *
 * Contact:  Qt Software Information (qt-info@nokia.com)**
 *
 * Commercial Usage
 * Licensees holding valid Qt Commercial licenses may use this file in 
 * accordance with the Qt Commercial License Agreement provided with the 
 * Software or, alternatively, in accordance with the terms contained in 
 * a written agreement between you and Nokia.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the 
 * GNU Lesser General Public License version 2.1 only as published by the 
 * Free Software Foundation and appearing in the file LICENSE.LGPL included 
 * in the packaging of this file.  Please review the following information 
 * to ensure the GNU Lesser General Public License version 2.1 requirements 
 * will be met:  http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the 
 * GNU General Public License version 3.0 as published by the Free Software 
 * Foundation and appearing in the file LICENSE.GPL included in the 
 * packaging of this file.  Please review the following information to ensure 
 * the GNU General Public License version 3.0 requirements will be 
 * met: http://www.gnu.org/copyleft/gpl.html.
 *
 * If you are unsure which license is appropriate for your use, please contact 
 * the sales department at qt-sales@nokia.com.
-->

<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method='html' version='2.0' encoding='UTF-8' indent='yes'/>
<xsl:template match="user">
<!DOCTYPE html>
<html>
<head>
<style>

body {
    background-color: #<xsl:value-of select="profile_background_color"/>;
    background-image: url(<xsl:value-of select="profile_background_image_url"/>);
    background-position: top left;
    background-attachment: fixed;
    <xsl:choose>
        <xsl:when test="profile_background_tile = 'true'">
            background-repeat: repeat;
        </xsl:when>
        <xsl:otherwise>
            background-repeat: no-repeat;
        </xsl:otherwise>
    </xsl:choose>
}

* {
    color: #<xsl:value-of select="profile_text_color"/>;
}

a {
    color:  #<xsl:value-of select="profile_link_color"/>;
}

header, nav, article, section, footer {
    display: block;
}

header {
    width: 100%;
    margin-bottom: 30px;
}

nav {
    margin: 0;
    padding: 0;
    float: left;
    width: 200px;
    background-color: #<xsl:value-of select="profile_sidebar_fill_color"/>;
    border: 1px solid #<xsl:value-of select="profile_sidebar_border_color"/>;
}

section {
    margin-left: 210px;
}

footer {
    margin-top: 30px;
    text-align: center;
    color: darkgray;
}

.navheader {
    padding: 3px 10px;
    font-size: 14pt;
    font-weight: bold;
}

nav ul {
    margin: 0;
    margin-bottom: 10px;
    padding: 0;
    padding-left: 10px;
}

nav li {
    list-style-type: none;
    margin-bottom: 5px;
}

img {
    border: 1px solid black;
}

#welcome {
    font-size: 16pt;
    font-weight: bold;
    display: inline;
}

#profile_image {
    float: left;
    margin-right: 10px;
    vertical-align: middle;
}

</style>

<title> Twitter Client Demo (using <abbr> XSLT </abbr> and <abbr> HTML5 </abbr> </title>
</head>

<body>
    <header>
        <a id="profile_image">
            <xsl:attribute name="href">http://twitter.com/<xsl:value-of select="screen_name"/></xsl:attribute>
            <img width="50" height="50">
                <xsl:attribute name="src"><xsl:value-of select="profile_image_url"/></xsl:attribute>
            </img>
        </a>

        <h1 id="welcome"> Welcome <xsl:value-of select="name"/> </h1>

        <div id="count"> You have <xsl:value-of select="friends_count"/> friends and <xsl:value-of select="followers_count"/> followers. </div>
    </header>

    <nav id="navigation"> 
        <div> 
            <div class="navheader"> Messages </div> 
            <ul>
                <li> <a href="#meAndMyFriendsStatus"> Me and My friends </a> </li>
                <li> <a href="#myStatus"> Me </a> </li>
                <li> <a href="#publicStatus"> Everyone </a> </li>
            </ul>
        </div>
        <div> <div class="navheader"> Friends </div> <div id="friends"> Loading... </div> </div>
        <div> <div class="navheader"> Followers </div> <div id="followers"> Loading... </div> </div>
    </nav>

    <section> 
        <div id="central"><h3>Loading...</h3>Please wait...</div>
    </section>

    <footer> Twitter Client Demo (using <abbr> XSLT </abbr> and <abbr> HTML5 </abbr>) </footer>
</body>

<script>
    function setCentralHtml(html, title) { 
        html = '<h3> ' + title + ' </h3>' + html;
        document.getElementById('central').innerHTML = html;
    }

    function setFriendsHtml(html) {
        var friends = document.getElementById('friends');
        friends.innerHTML = html;
    }

    function setFollowersHtml(html) {
        var followers = document.getElementById('followers');
        followers.innerHTML = html;
    }

</script>
</html>

</xsl:template>
</xsl:stylesheet>
