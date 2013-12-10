/** This file is part of VideoFeed**
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
 */

function Settings(name) {
    this.data = [];
    this.db = openDatabase(name, "1.0", name + " Settings", 200000);
    this.db.transaction(function (tx) {
                            tx.executeSql("CREATE TABLE IF NOT EXISTS Settings(key TEXT UNIQUE, value TEXT)", []);
                        });
    var self = this;
    this.db.transaction(function (tx) {
                            tx.executeSql("SELECT key, value FROM Settings", [], 
                                          function(tx, result) { self.processResult(tx, result); }, 
                                          function() { self.readError(tx, error); });
                        });

    return this;
}

Settings.prototype = {
    onready : function(data) { 
    },

    processResult : function(tx, result) {
        this.data = [];
        for (var i = 0; i < result.rows.length; i++) {
            var row = result.rows.item(i);
            this.data[row['key']] = row['value'];
        }
        this.onready(this.data);
    },

    readError : function(tx, error) {
        alert('Failed to read database : ' + error.message);
    }
};

Settings.prototype.value = function(key) {
    return this.data[key];
}

Settings.prototype.setValue = function(key, value) {
    this.data[key] = value;
}

Settings.prototype.remove = function(key) {
    delete this.data[key];
}

Settings.prototype.removeAll = function() {
    this.data = [];
}

Settings.prototype.clearDatabase = function() {
    this.db.transaction(function (tx) {
                            tx.executeSql("DELETE FROM Settings", []);
                        });
}

Settings.prototype.insert = function(key, value) {
    alert(key + ' ' + value);
}

Settings.prototype.commit = function() {
    this.clearDatabase();
    var self = this;

    this.db.transaction(function (tx) {
                            for (var key in self.data)
                                tx.executeSql("INSERT INTO Settings (key, value) VALUES (?, ?)", [key, self.data[key]]);
                        });
}

