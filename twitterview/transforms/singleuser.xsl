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

<xsl:template name="processText">
    <xsl:param name="text"/>
    <xsl:param name="in_reply_to"/>

    <xsl:choose>
        <xsl:when test="$in_reply_to = ''">
            <xsl:value-of select="$text"/>
        </xsl:when>
        <xsl:otherwise>
            <a>
                <xsl:attribute name="href">#friend_<xsl:value-of select="$in_reply_to"/></xsl:attribute>
                <xsl:value-of select="$in_reply_to"/>
            </a>
            <span> : <xsl:value-of select="substring-after(string($text), concat('@', $in_reply_to))"/> </span>
        </xsl:otherwise>
    </xsl:choose>

</xsl:template>

<xsl:template match="/">

<table border="0" width="100%" cellpadding="5" style="border-spacing: 5px">
<xsl:for-each select="statuses/status">
<tr style="margin-bottom: 10px; padding: 2px;">
    <td>
        <xsl:if test="position() mod 2 = 1">
            <xsl:attribute name="style">background-color: #DDFFCC</xsl:attribute>
        </xsl:if>

        <div>
            <xsl:if test="position() = 1">
                <xsl:attribute name="style"> font-size: 16pt </xsl:attribute>
            </xsl:if>
            <xsl:call-template name="processText">
                <xsl:with-param name="text" select="text"/>
                <xsl:with-param name="in_reply_to" select="in_reply_to_screen_name"/>
            </xsl:call-template>
        </div>

        <div style="font-size: 8pt; font-style: italic; margin-top: 10px"> <xsl:value-of select="created_at"/> </div>
    </td>
</tr>
</xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
