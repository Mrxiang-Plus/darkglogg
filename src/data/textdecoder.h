/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEXTDECODER_H
#define TEXTDECODER_H

#include <QByteArray>
#include <QString>

#include <memory>

// Minimal replacement for QTextCodec, which left QtCore in Qt 6 and only
// lives on in the optional Qt5Compat module. Covers exactly the encodings
// offered by the "Display encoding" menu: the Unicode ones go through
// QStringConverter, the legacy 8-bit ones (CP1251/CP1252/Big5/GB18030/
// Shift-JIS/KOI8-R) through MultiByteToWideChar on Windows and iconv
// elsewhere, so no extra dependency is introduced.
class TextDecoder {
 public:
  // Always returns a usable decoder; unknown names fall back to Latin-1.
  static std::unique_ptr<TextDecoder> codecForName(const QByteArray& name);

  QString toUnicode(const QByteArray& data) const;
  QString toUnicode(const char* data, int len) const;

  QByteArray name() const { return name_; }

 private:
  TextDecoder(const QByteArray& name, int winCodePage, bool qtNative);

  QByteArray name_;
  // Windows code page for MultiByteToWideChar (0 = not used).
  int winCodePage_;
  // True when QStringConverter/Latin-1 handles this encoding natively.
  bool qtNative_;
};

#endif
