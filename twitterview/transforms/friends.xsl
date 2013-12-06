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

<xsl:template match="/users">

<ul>
<xsl:for-each select="user">
<xsl:sort select="followers_count"/>
    <li>
        <a>
            <xsl:attribute name="href">#friend_<xsl:value-of select="screen_name"/></xsl:attribute>
            <xsl:attribute name="title"><xsl:value-of select="name"/></xsl:attribute>
            <img width="32" height="32" style="vertical-align: middle; margin-right: 10px">
                <xsl:attribute name="src"><xsl:value-of select="profile_image_url"/></xsl:attribute>
            </img>
            <xsl:value-of select="name"/>
        </a>
    </li>
</xsl:for-each>
</ul>

</xsl:template>
</xsl:stylesheet>
