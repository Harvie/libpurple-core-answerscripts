#
# core-answerscripts Makefile
#
# Copyright 2010 Thomas 'Harvie' Mudrunka <harvie AT email DOT cz>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CC := gcc
LIBTOOL := libtool

ifeq ($(PREFIX),)
  LIB_INSTALL_DIR = $(HOME)/.purple/plugins
else
  LIB_INSTALL_DIR = $(PREFIX)/lib/purple-2/
endif

PLUGIN_NAME = answerscripts

PURPLE_CFLAGS  = $(shell pkg-config purple --cflags)
PURPLE_LIBS    = $(shell pkg-config purple --libs)

all: $(PLUGIN_NAME).so

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	cp $(PLUGIN_NAME).so $(LIB_INSTALL_DIR)

$(PLUGIN_NAME).so: $(PLUGIN_NAME).o
	$(CC) -shared $(CFLAGS) $< -o $@ $(PURPLE_LIBS) $(GTK_LIBS) -Wl,--export-dynamic -Wl,-soname

$(PLUGIN_NAME).o:$(PLUGIN_NAME).c 
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PURPLE_CFLAGS) $(GTK_CFLAGS) -DHAVE_CONFIG_H

clean:
	rm -rf *.o *.c~ *.h~ *.so *.la .libs

user:
	cp -r purple/* $(HOME)/.purple/
	mv $(HOME)/.purple/$(PLUGIN_NAME).sh $(HOME)/.purple/$(PLUGIN_NAME).exe
