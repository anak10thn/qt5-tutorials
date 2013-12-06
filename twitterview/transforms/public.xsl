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

<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:fb="http://www.forwardbias.in">
<xsl:output method='html' version='2.0' encoding='UTF-8' indent='yes'/>

<xsl:template name="processText">
    <xsl:param name="text"/>

    <xsl:value-of select="$text"/>
</xsl:template>

<xsl:template match="/">

<form style="margin-bottom: 10px" onsubmit="view.setStatus(document.getElementById('status').value); return false;">
    <input type="text" value="What are you doing..." style="width: 450px" id="status" onfocus="this.value=''"></input>
    <input style="vertical-align: middle" type="submit" value="Update"/>
</form>

<table border="0" width="100%" cellspacing="0">
<xsl:for-each select="statuses/status">
<tr>
    <xsl:if test="position() mod 2 = 1">
        <xsl:attribute name="style">background: #DDFFCC</xsl:attribute>
    </xsl:if>

    <td rowspan="2" width="200px">
        <img style="float: left; margin: 3px; margin-right: 10px" width="50" height="50"><xsl:attribute name="src"><xsl:value-of select="user/profile_image_url"/></xsl:attribute></img>
        <a style="margin-right: 10px; font-weight: bold"> 
            <xsl:attribute name="href">#friend_<xsl:value-of select="user/screen_name"/></xsl:attribute>
            <xsl:value-of select="user/name"/> 
        </a>
    </td>

    <td>
        <xsl:call-template name="processText">
            <xsl:with-param name="text" select="text"/>
        </xsl:call-template>
    </td>
</tr>
<tr>
    <xsl:if test="position() mod 2 = 1">
        <xsl:attribute name="style">background: #DDFFCC</xsl:attribute>
    </xsl:if>

    <td valign="bottom" style="font-size: 8pt; font-style: italic">
        <div> <xsl:value-of select="created_at"/> </div>
    </td>
</tr>
</xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
